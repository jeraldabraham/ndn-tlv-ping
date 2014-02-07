Ping Application For NDN (ndn-tlv-ping)
======================================

This is an application tool developed over ndn-cpp APIs for TLV packet
based communication over NDN configured network. The NDN network has to
be first created with 'ndnx' and 'ndnd-tlv'. This is followed by environment
security setup with 'ndn-cpp-security-tools'. The application also requires
installation of ndn-cpp and CPP boost libraries. The installations need to
follow a strict order as the ndnd

To run the following must be ensured(FOLLOW ORDER STRICTLY)

1. Install ndnx (install all necessary dependencies)
2. Install CPP Boost Library > 1.47

        sudo apt-get install libboost1.48-all-dev

3. Install ndn-cpp-dev (install all necessary dependencies except boost)
4. Install ndnd-tlv install all necessary dependencies except boost)
5. Install and Configure Security Environment with ndn-cpp-security-tools

-----------------------------------------------------

## 1. Compile And Installation Instructions: ##

    git clone git://github.com/jeraldabraham/ndn-tlv-ping ndn-tlv-ping
    cd ndn-tlv-ping
    ./waf configure 
    ./waf
    sudo ./waf install

## 2. Tool Run Instructions & Command Line Options: ##

    Usage: ndnpingserver ndnx:/name/prefix [options]
    Starts a NDN ping server that responds to Interests with name ndnx:/name/prefix/ping/number.
        [-x freshness] - set FreshnessSeconds
        [-h] - print this message and exit


    Usage: ndnping ndnx:/name/prefix [options]
    Ping a NDN name prefix using Interests with namendnx:/name/prefix/ping/number.
    The numbers in the Interests are randomly generated unless specified.
        [-i interval] - set ping interval in seconds (minimum 1 second)
        [-c count] - set total number of pings
        [-n number] - set the starting number, the number is incremented by 1 after each Interest
        [-p identifier] - add identifier to the Interest names before the numbers to avoid conflict
        [-a] - allow routers to return ping Data from cache (allowed by default if NDNx version < 0.8.0)
        [-t] - print timestamp
        [-h] - print this message and exit


## 3. Sample Run Instructions ##

__ON MACHINE #1__

Start the ndnd-tlv daemon

        ndnd-tlv-start

Start ping server

        ndnpingserver /test/pingserver

__ON MACHINE #2__

Start the ndnd-tlv daemon

        ndnd-tlv-start

Start the ping client
        
        ndnping /test/pingserver

* Use command line options shown above to adjust ping configuration.
