/**
 *
 * Copyright (C) 2013 University of Arizona.
 * @author: Jerald Paul Abraham <jeraldabraham@email.arizona.edu>
 *
 */

#include <boost/asio.hpp>

#include <ndn-cpp-dev/name.hpp>
#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/node.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>

using namespace ndn;

class NdnTlvPingServer
{
public:

  NdnTlvPingServer( char* programName )
  {
    freshnessSeconds_ = getMinimumFreshnessSeconds();
    totalPings_ = 0;
    programName_ = programName;
    ioService_ = ptr_lib::make_shared<boost::asio::io_service>();
    face_ = Face(ioService_);
  }

  NdnTlvPingServer()
  : keyChain_()
  {
  }

  void
  usage()
  {
    std::cout << "\nUsage: " << programName_ << " ndnx:/name/prefix [options]\n"
        "Starts a NDN ping server that responds to Interests with name"
        " ndnx:/name/prefix/ping/number.\n"
        "  [-x freshness] - set FreshnessSeconds\n"
        "  [-h] - print this message and exit\n\n";
    exit(1);
  }

  int
  getMinimumFreshnessSeconds()
  {
    return 1;
  }

  void
  setFreshnessSeconds( int freshnessSeconds )
  {
    if (freshnessSeconds <= 0)
      usage();
    freshnessSeconds_ = freshnessSeconds;
  }

  void
  setPrefix( char* prefix )
  {
    prefix_ = prefix;
  }

  void
  onInterest( const ptr_lib::shared_ptr<const Name> &name, const ptr_lib::shared_ptr<const Interest> &interest )
  {
    Name interestName;
    interestName = interest->getName();
    if ( name_.isPrefixOf(interestName) ) {
      std::cout << "Interest Received";
      std::cout << " - Ping Reference = " << interestName.getSubName(interest->getName().size()-1).toUri().substr(1);
      std::cout << std::endl;
      char responseContent[] = "NDN TLV Ping Response";
      Data data(interestName);
      data.setFreshnessPeriod(freshnessSeconds_);
      data.setContent((const uint8_t*)responseContent, sizeof(responseContent));
      keyChain_.sign(data);
      face_.put(data);
      totalPings_++;
    }
  }

  void
  onRegisterFailed( const ptr_lib::shared_ptr<const Name>& )
  {
    std::cerr << "ERROR: Failed to register prefix in local hub's daemon" << std::endl;
    face_.shutdown();
  }

  void
  signalHandler()
  {
    std::cout << "\n\nTotal Ping Interests Processed = " << totalPings_ <<std::endl;
    std::cout << "Shutting Down Ping Server (" << name_.toUri() << ").\n" << std::endl;
    face_.shutdown();
    ioService_.reset();
    exit(1);
  }

  void
  initialize()
  {
    std::cout << "\n=== Ping Server " << prefix_ <<" ===\n" << std::endl;

    boost::asio::signal_set signalSet(*ioService_, SIGINT, SIGTERM);
    signalSet.async_wait(boost::bind(&NdnTlvPingServer::signalHandler, this));
    name_.set(prefix_);
    name_.append("ping");
    face_.setInterestFilter(name_,
                            func_lib::bind(&NdnTlvPingServer::onInterest, this, _1, _2),
                            func_lib::bind(&NdnTlvPingServer::onRegisterFailed, this, _1));
    try {
      face_.processEvents();
    }
    catch(std::exception &e) {
          std::cerr << "ERROR: " << e.what() << std::endl;
    }
  }

private:
  KeyChain keyChain_;
  int totalPings_;
  int freshnessSeconds_;
  char *programName_;
  char *prefix_;
  Face face_;
  Name name_;
  ptr_lib::shared_ptr<boost::asio::io_service> ioService_;
};

int main( int argc, char* argv[] )
{
  int res;

  NdnTlvPingServer ndnTlvPingServer (argv[0]);
  while ((res = getopt(argc, argv, "hdx:")) != -1) {
    switch (res) {
      case 'h'  :
        ndnTlvPingServer.usage();
        break;
      case 'x'  :
        ndnTlvPingServer.setFreshnessSeconds(atoi(optarg));
        break;
      default   :
        ndnTlvPingServer.usage();
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argv[0] == NULL)
    ndnTlvPingServer.usage();

  ndnTlvPingServer.setPrefix(argv[0]);
  ndnTlvPingServer.initialize();

  return 0;
}
