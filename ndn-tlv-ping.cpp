/**
 *
 * Copyright (C) 2013 University of Arizona.
 * @author: Jerald Paul Abraham <jeraldabraham@email.arizona.edu>
 *
 */

#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/name.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>

using namespace ndn;

class NdnTlvPing
{
public:

  NdnTlvPing( char* programName )
  {
    allowCaching_ = false;
    printTimestamp_ = false;
    totalPings_ = -1;
    startPingNumber_ = 1;
    pingsSent_ = 0;
    pingsReceived_ = 0;
    pingInterval_ = getPingMinimumInterval();
    clientIdentifier_ = 0;
    pingTimeoutThreshold_ = getPingTimeoutThreshold();
    programName_ = programName;
    ioService_ = ptr_lib::make_shared<boost::asio::io_service>();
    face_ = Face(ioService_);
  }

  class PingStatistics
  {
  public:

    PingStatistics()
    {
      sentPings_ = 0;
      receivedPings_ = 0;
      pingStartTime_ = boost::posix_time::microsec_clock::local_time();
      minimumRoundTripTime_ = std::numeric_limits<double>::max();
      maximumRoundTripTime_ = 0;
      averageRoundTripTimeData_ = 0;
      standardDeviationRoundTripTimeData_ = 0;
    }

    void
    addToPingStatistics(double roundTripTime)
    {
      if (roundTripTime < minimumRoundTripTime_)
        minimumRoundTripTime_ = roundTripTime;
      if (roundTripTime > maximumRoundTripTime_)
        maximumRoundTripTime_ = roundTripTime;
      averageRoundTripTimeData_ += roundTripTime;
      standardDeviationRoundTripTimeData_ += roundTripTime*roundTripTime;
    }

    int sentPings_;
    int receivedPings_;
    boost::posix_time::ptime pingStartTime_;
    double minimumRoundTripTime_;
    double maximumRoundTripTime_;
    double averageRoundTripTimeData_;
    double standardDeviationRoundTripTimeData_;

  };

  NdnTlvPing()
  : keyChain_()
  {
  }

  void
  usage()
  {
    std::cout << "\nUsage: " << programName_ << " ndnx:/name/prefix [options]\n"
        "Ping a NDN name prefix using Interests with name"
        "ndnx:/name/prefix/ping/number.\n"
        "The numbers in the Interests are randomly generated unless specified.\n"
        "  [-i interval] - set ping interval in seconds (minimum "
        << getPingMinimumInterval() << " second)\n"
        "  [-c count] - set total number of pings\n"
        "  [-n number] - set the starting number, the number is incremented by 1"
        " after each Interest\n"
        "  [-p identifier] - add identifier to the Interest names before the"
        " numbers to avoid conflict\n"
        "  [-a] - allow routers to return ping Data from cache"
        " (allowed by default if NDNx version < 0.8.0)\n"
        "  [-t] - print timestamp\n"
        "  [-h] - print this message and exit\n\n";
    exit(1);
  }

  double
  getPingMinimumInterval()
  {
    return 1;
  }

  int
  getPingTimeoutThreshold()
  {
    return 4000;
  }

  void
  setTotalPings( int totalPings )
  {
    if (totalPings <= 0)
      usage();
    totalPings_ = totalPings;
  }

  void
  setPingInterval( double pingInterval )
  {
    if (pingInterval < getPingMinimumInterval())
      usage();
    pingInterval_ = pingInterval;
  }

  void
  setStartPingNumber( int startPingNumber )
  {
    if (startPingNumber < 0)
      usage();
    startPingNumber_ = startPingNumber;
  }

  void
  setAllowCaching()
  {
    allowCaching_ = true;
  }

  void
  setPrintTimestamp()
  {
    printTimestamp_ = true;
  }

  void
  setClientIdentifier( char* clientIdentifier )
  {
    clientIdentifier_ = clientIdentifier;
    if (strlen(clientIdentifier) == 0)
      usage();
    while (*clientIdentifier != '\0') {
      if( isalnum(*clientIdentifier) == 0 )
        usage();
      clientIdentifier++;
    }
  }

  void
  setPrefix( char* prefix )
  {
    prefix_ = prefix;
  }

  void
  onData( ndn::Face &face,
          const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest,
          const ndn::ptr_lib::shared_ptr<ndn::Data> &data )
  {
    std::string pingReference;
    double roundTripTime;
    boost::posix_time::time_duration roundTripDuration;
    pingReference = interest->getName().toUri();
    pingsReceived_++;
    pingStatistics_.receivedPings_++;
    roundTripDuration = boost::posix_time::microsec_clock::local_time() - sentTimes_[pingReference];
    roundTripTime = roundTripDuration.total_microseconds()/1000.0;
    std::cout << "Content From " << prefix_;
    std::cout << " - Ping Reference = " << interest->getName().getSubName(interest->getName().size()-1).toUri().substr(1);
    std::cout << "  \t- Round Trip Time = " << roundTripTime << " ms" << std::endl;
    sentTimes_.erase(sentTimes_.find(pingReference));
    pingStatistics_.addToPingStatistics(roundTripTime);
  }

  void
  onTimeout( ndn::Face &face, const ndn::ptr_lib::shared_ptr<const ndn::Interest> &interest )
  {
    std::cout << "Timeout From " << prefix_;
    std::cout << " - Ping Reference = " << interest->getName().getSubName(interest->getName().size()-1).toUri().substr(1);
    std::cout << std::endl;
  }

  void
  printPingStatistics()
  {
    double packetLossPercentage;
    std::cout << "\n\n=== " << " Ping Statistics For "<< prefix_ <<" ===" << std::endl;
    std::cout << "Sent=" << pingStatistics_.sentPings_;
    std::cout << ", Received=" << pingStatistics_.receivedPings_;
    packetLossPercentage = pingStatistics_.sentPings_-pingStatistics_.receivedPings_;
    packetLossPercentage /= pingStatistics_.sentPings_;
    std::cout << ", Packet Loss=" << packetLossPercentage*100.0 << "%";
    std::cout << ", Total Time=" << pingStatistics_.averageRoundTripTimeData_ << " ms\n";
    if (pingStatistics_.receivedPings_ > 0) {
      double averageRoundTripTime;
      double standardDeviationRoundTripTime;
      averageRoundTripTime = pingStatistics_.averageRoundTripTimeData_ / pingStatistics_.receivedPings_;
      standardDeviationRoundTripTime = pingStatistics_.standardDeviationRoundTripTimeData_ / pingStatistics_.receivedPings_;
      standardDeviationRoundTripTime -= averageRoundTripTime*averageRoundTripTime;
      standardDeviationRoundTripTime = std::sqrt( standardDeviationRoundTripTime );
      std::cout << "Round Trip Time (Min/Max/Avg/MDev) = (";
      std::cout << pingStatistics_.minimumRoundTripTime_ << "/";
      std::cout << pingStatistics_.maximumRoundTripTime_ << "/";
      std::cout << averageRoundTripTime << "/";
      std::cout << standardDeviationRoundTripTime << ") ms\n";
    }
    std::cout << std::endl;
  }

  void
  performPing( const boost::system::error_code& errorCode, boost::asio::deadline_timer* deadlineTimer )
  {
    if ((totalPings_ < 0) || (pingsSent_ < totalPings_)) {
      pingsSent_++;
      pingStatistics_.sentPings_++;

      //Perform Ping
      int pingNumber;
      char pingNumberString[20];
      Name pingPacketName(prefix_);

      pingPacketName.append("ping");
      if(clientIdentifier_ != 0)
        pingPacketName.append(clientIdentifier_);
      std::memset(pingNumberString, 0, 20);
      if (startPingNumber_ < 0)
        pingNumber = (int)std::rand();
      else
        {
          pingNumber = startPingNumber_;
          startPingNumber_++;
        }
      sprintf(pingNumberString, "%d", pingNumber);
      pingPacketName.append(pingNumberString);

      ndn::Interest interest(pingPacketName);
      if (allowCaching_)
        interest.setMustBeFresh(false);
      else
        interest.setMustBeFresh(true);
      interest.setInterestLifetime(pingTimeoutThreshold_);
      interest.setNonce(pingNumber);
       try {
        sentTimes_[pingPacketName.toUri()] = boost::posix_time::microsec_clock::local_time();
        face_.expressInterest(interest,
                              ndn::func_lib::bind(&NdnTlvPing::onData, this, boost::ref(face_), _1, _2),
                              ndn::func_lib::bind(&NdnTlvPing::onTimeout, this, boost::ref(face_), _1));
        deadlineTimer->expires_at(deadlineTimer->expires_at() +
                                  boost::posix_time::millisec((int)(pingInterval_*1000)));
        deadlineTimer->async_wait(boost::bind(&NdnTlvPing::performPing,
                                              this,
                                              boost::asio::placeholders::error,
                                              deadlineTimer));
      }
      catch(std::exception &e) {
          std::cerr << "ERROR: " << e.what() << std::endl;
      }
    }
  }

  void
  signalHandler()
  {
    face_.shutdown();
    ioService_.reset();
    printPingStatistics();
    exit(1);
  }

  void
  initialize()
  {
    std::cout << "\n=== Pinging " << prefix_  << " ===\n" <<std::endl;

    boost::asio::signal_set signalSet(*ioService_, SIGINT, SIGTERM);
    signalSet.async_wait(boost::bind(&NdnTlvPing::signalHandler, this));

    boost::asio::deadline_timer deadlineTimer(*ioService_,
                                              boost::posix_time::seconds(pingInterval_));

    deadlineTimer.async_wait(boost::bind(&NdnTlvPing::performPing,
                                         this,
                                         boost::asio::placeholders::error,
                                         &deadlineTimer));
    try {
      face_.processEvents();
    }
    catch(std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
  }

private:

  KeyChain keyChain_;
  bool allowCaching_;
  bool printTimestamp_;
  int pingTimeoutThreshold_;
  int totalPings_;
  int pingsSent_;
  int pingsReceived_;
  int startPingNumber_;
  double pingInterval_;
  char* clientIdentifier_;
  char* programName_;
  char* prefix_;
  PingStatistics pingStatistics_;
  ptr_lib::shared_ptr<boost::asio::io_service> ioService_;
  Face face_;
  std::map<std::string, boost::posix_time::ptime> sentTimes_;
};


int main( int argc, char* argv[] )
{
  int res;

  NdnTlvPing ndnTlvPing (argv[0]);
  while ((res = getopt(argc, argv, "htai:c:n:p:")) != -1) {
    switch (res) {
      case 'a'  :
        ndnTlvPing.setAllowCaching();
        break;
      case 'c'  :
        ndnTlvPing.setTotalPings(atoi(optarg));
        break;
      case 'h'  :
        ndnTlvPing.usage();
        break;
      case 'i'  :
        ndnTlvPing.setPingInterval(atof(optarg));
        break;
      case 'n'  :
        ndnTlvPing.setStartPingNumber(atoi(optarg));
        break;
      case 'p'  :
        ndnTlvPing.setClientIdentifier(optarg);
        break;
      case 't'  :
        ndnTlvPing.setPrintTimestamp();
        break;
      default   :
        ndnTlvPing.usage();
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argv[0] == NULL)
    ndnTlvPing.usage();

  ndnTlvPing.setPrefix(argv[0]);
  ndnTlvPing.initialize();

  return 0;
}

