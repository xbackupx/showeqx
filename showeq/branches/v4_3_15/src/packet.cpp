/*
 * packet.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 */

/* Implementation of Packet class */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <netdb.h>

#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>

#include "everquest.h"
#include "packet.h"
#include "util.h"
#include "main.h"
#include "vpacket.h"
#include "logger.h"

//----------------------------------------------------------------------
// Macros

//#define DEBUG_PACKET
//#undef DEBUG_PACKET

// The following defines are used to diagnose packet handling behavior
// this define is used to diagnose packet processing (in dispatchPacket mostly)
//#define PACKET_PROCESS_DIAG 3 // verbosity level 0-n

// this define is used to diagnose cache handling (in dispatchPacket mostly)
//#define PACKET_CACHE_DIAG 3 // verbosity level (0-n)

// diagnose structure size changes
//#define PACKET_PROCESS_FRAG_DIAG

// define this to be pedantic about what packets are acceptable (no CRC errors)
#define PACKET_PEDANTIC 2

// diagnose structure size changes
#define PACKET_PAYLOAD_SIZE_DIAG 1

// Packet version is a unique number that should be bumped every time packet
// structure (ie. encryption) changes.  It is checked by the VPacket feature
// (currently the date of the last packet structure change)
#define PACKETVERSION  40100

//----------------------------------------------------------------------
// constants

// Arbitrary cutoff used to determine the relationship between
// an arq sequence may be from a wrap and not just be really old.
const int16_t arqSeqWrapCutoff = 1024;

// Arbitrary cutoff used to determine when to give up on seeing
// an arq sequenced packet, and just move on...
const int16_t arqSeqGiveUp = 256;


const in_port_t WorldServerGeneralPort = 9000;
const in_port_t WorldServerChatPort = 9876;
const in_port_t LoginServerMinPort = 15000;
const in_port_t LoginServerMaxPort = 15010;
const in_port_t ChatServerPort = 5998;

//----------------------------------------------------------------------
// static variables
#ifdef PACKET_CACHE_DIAG
static size_t maxServerCacheCount = 0;
#endif

// used to translate EQStreamID to a string for debug and reporting
static const char* const EQStreamStr[] = {"client-world", "world-client", "client-zone", "zone-client"};

//----------------------------------------------------------------------
// forward declarations
char *getTime (char *pchTime);

//----------------------------------------------------------------------
// Here begins the code
void searchForNum(unsigned char *data, int len, char *Name, char value){
    for(int n = 0; n< len; n++)
        if (data[n] == value)
            printf("%s contains %i at %i\n", Name, value, n);
}

#ifdef PACKET_PAYLOAD_SIZE_DIAG
bool validatePayloadSize(int len, int size, uint16_t code,
			 const char* clarifier,
			 const char* codeName, const char* structName)
{
  // verify size
  if (len != size)
  {
    fprintf(stderr, "WARNING: %s%s (%04x) (dataLen:%d != sizeof(%s):%d)!\n",
	    clarifier, codeName, code, len, structName, size);
    return false;
  }
  return true;
}

#define ValidatePayload(codeName, structName) \
  validatePayloadSize(len, sizeof( structName ), codeName, \
		      "", #codeName , #structName )
#endif

//----------------------------------------------------------------------
// EQPacketFormatRaw class methods
QString EQPacketFormatRaw::headerFlags(const QString& prefix, 
				       bool brief) const
{
  QString tmp;
  if (!prefix.isEmpty())
  {
    if (brief)
      tmp = prefix + ": ";
    else
      tmp = prefix + "[Hdr (" + QString::number(flagsHi(), 16) + ", "
	+ QString::number(flagsLo(), 16) + "): ";
  }
  else if (!brief)
    tmp = "[Hdr (" + QString::number(flagsHi(), 16) + ", "
	+ QString::number(flagsLo(), 16) + "): ";

  if (isARQ())
    tmp += "ARQ, ";
  if (isClosingLo())
    tmp += "closingLo, ";
  if (isFragment())
    tmp += "Fragment, ";
  if (isASQ())
    tmp += "ASQ, ";
  if (isSEQStart())
    tmp += "SEQStart, ";
  if (isClosingHi())
    tmp += "closingHi, ";
  if (isSEQEnd())
    tmp += "SEQEnd, ";
  if (isARSP())
    tmp += "ARSP, ";
  if (isNAK())
    tmp += "NAK, ";
  if (isSpecARQ())
    tmp += "SpecARQ, ";
  if (m_flagsHi.m_unknown1)
    tmp += "HiUnknown1, ";

  if (skip() != 0)
    tmp += QString("Skip: ") + QString::number(skip(), 16) + ", ";
  
  tmp += QString("seq: ") + QString::number(seq(), 16);

  tmp += "] ";

  if (!brief)
  {
    if (isARQ())
      tmp += QString("[ARQ: ") + QString::number(arq(), 16) + "] ";
    
    if (isFragment())
    {
      tmp += QString("[FragSeq: ") + QString::number(fragSeq(), 16)
	+ ", FragCur: " + QString::number(fragCur(), 16)
	+ ", FragTot: " + QString::number(fragTot(), 16) + "] ";
    }
  }

  return tmp;
}

//----------------------------------------------------------------------
// EQPacketFormat class methods
EQPacketFormat::EQPacketFormat(EQPacketFormat& packet, bool copy)
{
  if (!copy)
  {
    // the data is someone elses memory
    m_ownCopy = false;

    // just copy all their values
    m_packet = packet.m_packet;
    m_length = packet.m_length;
    m_postSkipData = packet.m_postSkipData;
    m_postSkipDataLength = packet.m_postSkipDataLength;
    m_payload = packet.m_payload;
    m_payloadLength = packet.m_payloadLength;
    m_arq = packet.m_arq;
  }
  else
  {
    init(packet.m_packet, packet.m_length, copy);
  }
}

EQPacketFormat::~EQPacketFormat()
{
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;
}

EQPacketFormat& EQPacketFormat::operator=(const EQPacketFormat& packet)
{
  // if this was a deep copy, delete the existing data
  if (m_ownCopy)
    delete [] (uint8_t*)m_packet;

  // initialize as deep as this object previously was
  init(packet.m_packet, packet.m_length, m_ownCopy);

  return *this;
}

void EQPacketFormat::init(EQPacketFormatRaw* packet,
			  uint16_t length,
			  bool copy)
{
  if (!copy)
  {
    // the data is someone elses memory
    m_ownCopy = false;
    
    // just copy the pointer
    m_packet = packet;

    // save the length
    m_length = length;
  }
  else
  {
    // this is our own copy
    m_ownCopy = true;

    // allocate memory for the copy
    // NOTE: We should use an allocater for this instead of normal new
    m_packet = (EQPacketFormatRaw*)new uint8_t[length];

    // copy the data
    memcpy((void*)m_packet, (void*)packet, length);

    // save the length
    m_length = length;
  }

  // finish initialization
  init();
}

void EQPacketFormat::init()
{
  // calculate position of first byte past the skipable data
  m_postSkipData = ((uint8_t*)m_packet) + m_packet->skip() + 2;

  // calculate the length of the rest of the data
  m_postSkipDataLength = m_length - (m_postSkipData - ((uint8_t*)m_packet));

  // get the location of the payload
  m_payload = m_packet->payload();

  // calculate the lenght of the payload (length - diff - len(checkSum))
  m_payloadLength = m_length - (m_payload - ((uint8_t*)m_packet)) - 4; 
    

  // make a local copy of the arq to speed up comparisons
  m_arq = eqntohuint16(&m_postSkipData[2]);
}

QString EQPacketFormat::headerFlags(const QString& prefix, 
				    bool brief) const
{
  QString tmp;

  if (brief)
    tmp = prefix + ": ";
  else
    tmp = prefix + " Hdr (" + QString::number(flagsHi(), 16) + ", "
      + QString::number(flagsLo(), 16) + "): ";

  tmp += m_packet->headerFlags("", true);

  if (!brief)
  {
    if (isARQ())
      tmp += QString("arq: ") + QString::number(arq(), 16) + ", ";
    
    if (isFragment())
    {
      tmp += QString("FragSeq: ") + QString::number(fragSeq(), 16)
	+ ", FragCur: " + QString::number(fragCur(), 16)
	+ ", FragTot: " + QString::number(fragTot(), 16) + ", ";
    }

    tmp += QString("Opcode: ") + QString::number(eqntohuint16(payload()), 16);
  }

  return tmp;
}

//----------------------------------------------------------------------
// EQUDPIPPacketFormat class methods
EQUDPIPPacketFormat::EQUDPIPPacketFormat(uint8_t* data, 
					 uint32_t length, 
					 bool copy)
{
  uint8_t* ipdata;
  if (copy)
  {
    // allocate our own copy
    ipdata = new uint8_t[length];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)data, length);
  }
  else // just use the data that was passed in
    ipdata = data;

  // note whether or not this object ownw the memory
  m_ownCopy = copy;

  // initialize the object
  init(ipdata);
}

EQUDPIPPacketFormat::EQUDPIPPacketFormat(EQUDPIPPacketFormat& packet,
					 bool copy)
{
  // note whether or not this object ownw the memory
  m_ownCopy = copy;

  if (copy)
  {
    // allocate our own copy
    uint8_t* ipdata = new uint8_t[packet.m_dataLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_dataLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_dataLength = packet.m_dataLength;
    m_ip = packet.m_ip;
    m_udp = packet.m_udp;
  }
}

EQUDPIPPacketFormat::~EQUDPIPPacketFormat()
{
  if (m_ownCopy && (m_ip != NULL))
    delete [] (uint8_t*)m_ip;
}

EQUDPIPPacketFormat& EQUDPIPPacketFormat::operator=(const EQUDPIPPacketFormat& packet)
{
  if (m_ownCopy && (m_ip != NULL))
    delete [] (uint8_t*)m_ip;

  if (m_ownCopy)
  {
    // allocate our own copy
    uint8_t* ipdata = new uint8_t[packet.m_dataLength];

    // copy the data into the copy
    memcpy((void*)ipdata, (void*)packet.m_ip, packet.m_dataLength);

    // initialize the object
    init(ipdata);
  }
  else
  {
    // just copy over the other objects data
    m_dataLength = packet.m_dataLength;
    m_ip = packet.m_ip;
    m_udp = packet.m_udp;
  }

  return *this;
}

void EQUDPIPPacketFormat::init(uint8_t* data)
{
  uint32_t ipHeaderLength, length;

  // we start at the IP header
  m_ip = (struct ip*)data;

  // retrieve the total length from the header
  m_dataLength = ntohs (m_ip->ip_len);

  // use this length to caclulate the rest
  length = m_dataLength;

  // skip past the IP header
  ipHeaderLength  = m_ip->ip_hl * 4;
  length  -= ipHeaderLength;
  data += ipHeaderLength;

  // get the UDP header
  m_udp   = (struct udphdr *) data;

  // skip over UDP header
  length  -= sizeof  (struct udphdr);
  data += (sizeof (struct udphdr));
  m_rawpayload = data;

  // initialize underlying EQPacketFormat with the UDP payload
  EQPacketFormat::init((EQPacketFormatRaw*)data, length, false);
}

QString EQUDPIPPacketFormat::headerFlags(bool brief) const
{
  QString tmp;
  tmp.sprintf("[%s:%d -> %s:%d]", 
	      (const char*)getIPv4SourceA(), getSourcePort(),
	      (const char*)getIPv4DestA(), getDestPort());

  return EQPacketFormat::headerFlags(tmp, brief);
}

//----------------------------------------------------------------------
// EQPacket class methods

/* EQPacket Class - Sets up packet capturing */
EQPacket::EQPacket (QObject * parent, const char *name)
  : QObject (parent, name),
    m_packetCapture(NULL),
    m_vPacket(NULL),
    m_timer(NULL),
    m_busy_decoding(false)
{
  m_timeDateFormat = "MMM dd yyyy hh:mm:ss:zzz";

   for (int a = 0; a < MAXSTREAMS + 1; a++)
   {
       m_serverCache[a] = new EQPacketMap;
       m_fragmentData[a] = NULL;
       m_serverArqSeqExp[a] = 0;
       m_serverArqSeqFound[a] = FALSE;
       m_fragmentDataSize[a] = 0;
       m_fragmentDataAllocSize[a] = 0;
       m_fragmentSeq[a] = 0;
       m_fragmentCur[a] = 0;
       m_packetCount[a] = 0;
   }

   m_session_tracking_enabled = showeq_params->session_tracking;
   m_eqstreamid = unknown_stream;
   m_clientPort = 0;
   m_serverPort = 0;

   struct hostent *he;
   struct in_addr  ia;
   if (showeq_params->ip.isEmpty() && showeq_params->mac_address.isEmpty())
   {
      printf ("No address specified\n");
      exit(0);
   }

   if (!showeq_params->ip.isEmpty())
   {
      /* Substitute "special" IP which is interpreted 
         to set up a different filter for picking up new sessions */

     if (showeq_params->ip == "auto")
	inet_aton (AUTOMATIC_CLIENT_IP, &ia);
     else if (inet_aton (showeq_params->ip, &ia) == 0)
     {
       he = gethostbyname(showeq_params->ip);
       if (!he)
       {
	 printf ("Invalid address; %s\n", (const char*)showeq_params->ip);
	 exit (0);
       }
       memcpy (&ia, he->h_addr_list[0], he->h_length);
     }
     m_client_addr = ia.s_addr;
     showeq_params->ip = inet_ntoa(ia);
     
     if (showeq_params->ip ==  AUTOMATIC_CLIENT_IP)
     {
       m_detectingClient = true;
       printf("Listening for first client seen.\n");
     }
     else
     {
       m_detectingClient = false;
       printf("Listening for client: %s\n",
	      (const char*)showeq_params->ip);
     }
   }

   if (!showeq_params->playbackpackets)
   {
      // create the pcap object and initialize, either with MAC or IP
      m_packetCapture = new PacketCaptureThread();
      if (showeq_params->mac_address.length() == 17)
         m_packetCapture->start(showeq_params->device, 
				showeq_params->mac_address, 
				showeq_params->realtime, MAC_ADDRESS_TYPE );
      else
         m_packetCapture->start(showeq_params->device,
                                showeq_params->ip, 
				showeq_params->realtime, IP_ADDRESS_TYPE );
   }

   for (int i = 0; i < MAXSTREAMS + 1 ; i++)
   {
       m_serverArqSeqFound[i] = false;
       m_serverArqSeqExp[i]  = 0;
   }
   m_serverPort       = 0;
   m_clientPort       = 0;

   /* Initialize the time of day structure */
   memset( &m_eqTime, 0x00, sizeof(struct eqTimeOfDay) );

   m_viewUnknownData = false;

   /* Create timer object */
   m_timer = new QTimer (this);

   if (!showeq_params->playbackpackets)
       connect (m_timer, SIGNAL (timeout ()), this, SLOT (processPackets ()));
   else
       connect (m_timer, SIGNAL (timeout ()), this, SLOT (processPlaybackPackets ()));
     
   /* setup VPacket */
   m_vPacket = NULL;

   QString section = "VPacket";
   // First param to VPacket is the filename
   // Second param is playback speed:  0 = fast as poss, 1 = 1X, 2 = 2X etc
   if (pSEQPrefs->isPreference("Filename", section))
   {
       const char *filename = pSEQPrefs->getPrefString("Filename", section);

       if (showeq_params->recordpackets)
       {
           m_vPacket = new VPacket(filename, 1, true);
           // Must appear befire next call to getPrefString, which uses a static string
           printf("Recording packets to '%s' for future playback\n", filename);

           if (pSEQPrefs->getPrefString("FlushPackets", section))
           m_vPacket->setFlushPacket(true);
       }

       else if (showeq_params->playbackpackets)
       {
          m_vPacket = new VPacket(filename, 1, false);
          m_vPacket->setCompressTime(pSEQPrefs->getPrefInt("CompressTime", section, 0));
          m_vPacket->setPlaybackSpeed(showeq_params->playbackspeed);

          printf("Playing back packets from '%s' at speed '%d'\n", filename,

          showeq_params->playbackspeed);
       }
   }
   else
   {
       showeq_params->recordpackets   = 0;
       showeq_params->playbackpackets = 0;
   }

   InitializeOpCodeMonitor();

   // a sanity check, if the user set it to below 32, they're prolly nuts
   if (showeq_params->arqSeqGiveUp >= 32)
       m_serverArqSeqGiveUp = showeq_params->arqSeqGiveUp;
   else
       m_serverArqSeqGiveUp = 32;


#if HAVE_LIBEQ || 1
   if (showeq_params->broken_decode)
       printf("Running with broken decode set to true.\n");
#else
   printf("Running without libEQ.cpp linked in.\n");
#endif
}

EQPacket::~EQPacket()
{
#ifdef PACKET_CACHE_DIAG
  printf("SEQ: Maximum Packet Cache Used: %d\n", maxServerCacheCount);
#endif


  if (m_packetCapture != NULL)
  {
    // stop any packet capture 
    m_packetCapture->stop();

    // delete the object
    delete m_packetCapture;
  }

  // try to close down VPacket cleanly
  if (m_vPacket != NULL)
  {
    // make sure any data is flushed to the file
    m_vPacket->Flush();

    // delete VPacket
    delete m_vPacket;
  }

  if (m_timer != NULL)
  {
    // make sure the timer is stopped
    m_timer->stop();

    // delete the timer
    delete m_timer;
  }

  resetEQPacket();
}

void EQPacket::InitializeOpCodeMonitor (void)
{
  do
  {
    if (!showeq_params->monitorOpCode_Usage)
      break;

    QString qsMonitoredOpCodes = showeq_params->monitorOpCode_List;

    if (qsMonitoredOpCodes == "0") /* DISABLED */
      break;

    if (qsMonitoredOpCodes.isEmpty())
      break;

    QString qsCommaBuffer (""); /* Construct these outside the loop so we don't have to construct
                                   and destruct these variables during each iteration... */
    QString qsColonBuffer ("");

    int            iCommaPos      = 0;
    int            iColonPos      = 0;
    uint8_t        uiIterationID  = 0;

    for (uint8_t uiIndex = 0; (uiIndex < OPCODE_SLOTS) && !qsMonitoredOpCodes.isEmpty(); uiIndex ++)
    {
      /* Initialize the variables with their default values */
      MonitoredOpCodeList      [uiIndex] [0] = 0; /* OpCode number (16-bit HEX) */
      MonitoredOpCodeList      [uiIndex] [1] = 3; /* Direction, DEFAULT: Client <--> Server */
      MonitoredOpCodeList      [uiIndex] [2] = 1; /* Show raw data if packet is marked as known */
      MonitoredOpCodeAliasList [uiIndex]     = "Monitored OpCode"; /* Name used when displaying the raw data */

      iCommaPos = qsMonitoredOpCodes.find (",");

      if (iCommaPos == -1)
        iCommaPos = qsMonitoredOpCodes.length ();

      qsCommaBuffer = qsMonitoredOpCodes.left (iCommaPos);
      qsMonitoredOpCodes.remove (0, iCommaPos + 1);


      uiIterationID = 0;


      uint8_t uiColonCount = qsCommaBuffer.contains(":");
      iColonPos = qsCommaBuffer.find (":");

      if (iColonPos == -1)
        qsColonBuffer  = qsCommaBuffer;

      else
        qsColonBuffer = qsCommaBuffer.left (iColonPos);

      while (uiIterationID <= uiColonCount)
      {
        if (uiIterationID == 0)
          MonitoredOpCodeList [uiIndex] [0] = qsColonBuffer.toUInt (NULL, 16);

        else if (uiIterationID == 1)
          MonitoredOpCodeAliasList [uiIndex] = qsColonBuffer;

        else if (uiIterationID == 2)
          MonitoredOpCodeList [uiIndex] [1] = qsColonBuffer.toUInt (NULL, 10);

        else if (uiIterationID == 3)
          MonitoredOpCodeList [uiIndex] [2] = qsColonBuffer.toUInt (NULL, 10);

        qsCommaBuffer.remove (0, iColonPos + 1);

        iColonPos = qsCommaBuffer.find (":");

        if (iColonPos == -1)
          qsColonBuffer = qsCommaBuffer;

        else
            qsColonBuffer = qsCommaBuffer.left (iColonPos);

        uiIterationID ++;
      }

#if 1 // ZBTEMP
      fprintf(stderr, "opcode=%04x name='%s' dir=%d known=%d\n",
	      MonitoredOpCodeList [uiIndex] [0],
	      (const char*)MonitoredOpCodeAliasList [uiIndex],
	      MonitoredOpCodeList [uiIndex] [1],
	      MonitoredOpCodeList [uiIndex] [2]);
#endif
    }
  } while ( "I like to use" == "break for flow control" );
}

/* Returns string representation of numeric IP address */
QString EQPacket::print_addr (in_addr_t  addr)
{
#ifdef DEBUG_PACKET
   debug ("print_addr()");
#endif /* DEBUG_PACKET */
  QString paddr;

  if (addr == m_client_addr)
    paddr = "client";
  else
  {
    paddr.sprintf ( "%d.%d.%d.%d",
                    addr & 0x000000ff,
                    (addr & 0x0000ff00) >> 8,
                    (addr & 0x00ff0000) >> 16,
                    (addr & 0xff000000) >> 24
                  );
   }

   return paddr;
}

/* Start the timer to process packets */
void EQPacket::start (int delay)
{
#ifdef DEBUG_PACKET
   debug ("start()");
#endif /* DEBUG_PACKET */
   m_timer->start (delay, false);
}

/* Stop the timer to process packets */
void EQPacket::stop (void)
{
#ifdef DEBUG_PACKET
   debug ("stop()");
#endif /* DEBUG_PACKET */
   m_timer->stop ();
}

/* Reads packets and processes waiting packets */
void EQPacket::processPackets (void)
{
  /* Make sure we are not called while already busy */
  if (m_busy_decoding)
     return;

  /* Set flag that we are busy decoding */
  m_busy_decoding = true;
  
  unsigned char buffer[BUFSIZ]; 
  short size;
  
  /* fetch them from pcap */
  while ((size = m_packetCapture->getPacket(buffer)))
  {
    /* Now.. we know the rest is an IP udp packet concerning the
     * host in question, because pcap takes care of that.
     */
      
    /* Now we assume its an everquest packet */
    if (showeq_params->recordpackets)
    {
      time_t now = time(NULL);
      m_vPacket->Record((const char *) buffer, size, now, PACKETVERSION);
    }
      
    dispatchPacket (size - sizeof (struct ether_header),
		  (unsigned char *) buffer + sizeof (struct ether_header) );
  }

  /* Clear decoding flag */
  m_busy_decoding = false;
}

/* Reads packets and processes waiting packets */
void EQPacket::processPlaybackPackets (void)
{
#ifdef DEBUG_PACKET
//   debug ("processPackets()");
#endif /* DEBUG_PACKET */
  /* Make sure we are not called while already busy */
  if (m_busy_decoding)
    return;

  /* Set flag that we are busy decoding */
  m_busy_decoding = true;

  unsigned char  buffer[8192];
  int            size;

  /* in packet playback mode fetch packets from VPacket class */
  time_t now;
  int timein = mTime();
  int i = 0;
    
  long version = PACKETVERSION;
  
  // decode packets from the playback buffer
  do
  {
    size = m_vPacket->Playback((char *) buffer, sizeof(buffer), &now, &version);
    
    if (size)
    {
      i++;
	
      if (PACKETVERSION == version)
      {
	dispatchPacket ( size - sizeof (struct ether_header),
		       (unsigned char *) buffer + sizeof (struct ether_header)
		       );
      }
      else
      {
	fprintf( stderr, "Error:  The version of the packet stream has " \
		 "changed since '%s' was recorded - disabling playback\n",
		 m_vPacket->getFileName());

	// stop the timer, nothing more can be done...
	stop();

	break;
      }
    }
    else
      break;
  } while ( (mTime() - timein) < 100);

  // check if we've reached the end of the recording
  if (m_vPacket->endOfData())
  {
    fprintf(stderr, "End of playback file '%s' reached.\n"
	    "Playback Finished!\n",
	    m_vPacket->getFileName());

    // stop the timer, nothing more can be done...
    stop();
  }

  /* Clear decoding flag */
  m_busy_decoding = false;
}

void EQPacket::logRawData ( const char   *filename,
                            unsigned char *data,
                            unsigned int   len
                          )
{
#ifdef DEBUG_PACKET
   debug ("logRawData()");
#endif /* DEBUG_PACKET */

   int file;
   //printf("FilePath: %s\n", fname);
   file = open (filename, O_CREAT | O_APPEND | O_RDWR);

   if (-1 == file)
   {
      fprintf(stderr,"\aUnable to open file: [%s]\n", filename);
      return;
   }

   write(file, data, len);

   close (file);
}

/* Makes a note in a log */
bool EQPacket::logMessage(const QString& filename,
			  const QString& message)
{
#ifdef DEBUG_PACKET
   debug ("logMessage()");
#endif /* DEBUG_PACKET */

   FILE *lh;

   //printf("FilePath: %s\n", fname);
   lh = fopen ((const char*)filename, "a");

   if (lh == NULL)
   {
      fprintf(stderr, "\aUnable to open file: [%s]\n", 
	      (const char*)filename);
      return false;
   }

   fprintf (lh, "%s\n", (const char*)message);

   fclose (lh);

   return true;
}

/* Logs packet data in a human-readable format */
bool EQPacket::logData ( const QString& filename,
			 uint32_t       len,
			 const uint8_t* data,
			 const QString& prefix)
{
#ifdef DEBUG_PACKET
   debug ("logData()");
#endif /* DEBUG_PACKET */

   FILE *lh;

   lh = fopen ((const char*)filename, "a");

   if (lh == NULL)
   {
      fprintf(stderr, "\aUnable to open file: [%s]\n", 
	      (const char*)filename);
      return false;
   }

   time_t now;
   now = time (NULL);

   if (!prefix.isEmpty())
     fprintf(lh, "%s ", (const char*)prefix.utf8());

   fprintf(lh, "[Size: %d] %s", len, ctime (&now));

   // make sure there is a len before attempting to print it
   if (len)
     fprintData(lh, len, data);
   else
     fprintf(lh, "\n");

   fclose (lh);

   return true;
}

inline QString opCodeToString(uint16_t opCode)
{
  QString tempStr;
  tempStr.sprintf("[OPCode: %#.04x]", opCode);

  if (opCode & FLAG_DECODE)
  {
    tempStr += " ";
    if (opCode & FLAG_COMP)
      tempStr += "[FLAG_COMP]";
    if (opCode & FLAG_COMBINED)
      tempStr += "[FLAG_COMBINED]";
    if (opCode & FLAG_CRYPTO)
      tempStr += "[FLAG_CRYPTO]";
    if (opCode & FLAG_IMPLICIT)
      tempStr += "[FLAG_IMPLICIT]";
  }

  return tempStr;
}

/* Logs packet data in a human-readable format */
bool EQPacket::logData ( const QString& filename,
			 uint32_t       len,
			 const uint8_t* data,
			 uint8_t        dir,
			 uint16_t       opcode,
			 const QString& origPrefix)
{
  FILE *lh;

  lh = fopen ((const char*)filename, "a");

  if (lh == NULL)
  {
    fprintf(stderr, "\aUnable to open file: [%s]\n", 
	    (const char*)filename);
    return false;
  }

  QFile logFile;
  if (!logFile.open(IO_Append | IO_WriteOnly, lh))
  {
    fclose(lh);
    return false;
  }

  QTextStream out(&logFile);

  // timestamp
  out << QDateTime::currentDateTime().toString(m_timeDateFormat) << " ";

  // append direction and opcode information
  if (!origPrefix.isEmpty())
    out << origPrefix << " ";
  
  // data direction and size
  out << ((dir == DIR_SERVER) ? "[Server->Client] " : "[Client->Server] ")
      << "[Size: " << QString::number(len) << "]" << endl;

  // print opcode info
  out << opCodeToString(opcode) << endl;

  // make sure there is a len before attempting to print it
  if (len)
    fprintData(lh, len, data);
  else
    out << endl;

  fclose (lh);

  return true;
}

bool EQPacket::logData(const QString& filename,
		       const EQUDPIPPacketFormat& packet)
{
  FILE *lh;

  lh = fopen ((const char*)filename, "a");

  if (lh == NULL)
  {
    fprintf(stderr, "\aUnable to open file: [%s]\n", 
	    (const char*)filename);
    return false;
  }

  QFile logFile;
  if (!logFile.open(IO_Append | IO_WriteOnly, lh))
  {
    fclose(lh);
    return false;
  }

  QTextStream out(&logFile);

  QString sourceStr, destStr;
  if (packet.getIPv4SourceN() == m_client_addr)
    sourceStr = "client";
  else
    sourceStr = packet.getIPv4SourceA();

  if (packet.getIPv4DestN() == m_client_addr)
    destStr = "client";
  else
    destStr = packet.getIPv4DestA();
  
  out << QDateTime::currentDateTime().toString(m_timeDateFormat)
      << " [" << sourceStr << ":" << QString::number(packet.getSourcePort())
      << "->" << destStr << ":" << QString::number(packet.getDestPort()) 
      << "] [Size: " << QString::number(packet.getRawPacketLength()) << "]"
      << endl;

  const EQPacketFormatRaw* raw = packet.getRawPacket();
  if (raw)
  {
    out << raw->headerFlags(QString(), false) << endl;
  }

  if (packet.payloadLength() >= 2)
  {
    QString tempStr;
    uint16_t opCode = eqntohuint16(packet.payload());
    out << opCodeToString(opCode) << endl;

    logFile.flush();
  }

#ifdef PACKET_PEDANTIC
  uint32_t crc32 = packet.calcCRC32();
  if (crc32 != packet.crc32())
  {
    out << "[BAD CRC32 (" << QString::number(crc32, 16) 
	<< " != " << QString::number(packet.crc32()) << ") ]" << endl;
  }
#endif

  logFile.flush();

  // make sure there is a len before attempting to print it
  if (packet.getRawPacketLength())
    fprintData(lh, packet.getRawPacketLength(), 
	       (const uint8_t*)packet.getRawPacket());
  else
    out << endl;

  fclose(lh);

  return true;
}

/* Logs packet data in a human-readable format */
bool EQPacket::logData( const QString& filename,
			uint32_t       len,
			const uint8_t* data,
			in_addr_t      saddr,
			in_addr_t      daddr,
			in_port_t      sport,
			in_port_t      dport
			)
{
#ifdef DEBUG_PACKET
   debug ("logData()");
#endif /* DEBUG_PACKET */

   QString prefix;
   if (saddr)
   {
     prefix.sprintf("%s:%d->%s:%d ",
		    (const char*)QString(print_addr (saddr)), sport,
		    (const char*)QString(print_addr (daddr)), dport);
   }

   return logData(filename, len, data, prefix);
}

/* This function decides the fate of the Everquest packet */
/* and dispatches it to the correct packet handling function */
void EQPacket::dispatchPacket (int size, unsigned char *buffer)
{
#ifdef DEBUG_PACKET
  debug ("dispatchPacket()");
#endif /* DEBUG_PACKET */
  /* Setup variables */

  EQUDPIPPacketFormat packet(buffer, size, false);

  
  if (showeq_params->logAllPackets)
  {
#if 1
    if (!logData(showeq_params->GlobalLogFilename, 
		 packet))
    {
      /* Problem writing to logs, untoggle the GUI checkmark to
	 show that logging is disabled. */
      emit toggle_log_AllPackets();
    }
#else
    if (!logData(showeq_params->GlobalLogFilename, 
		 packet.getRawPacketLength(), 
		 (const uint8_t*)packet.getRawPacket(), 
		 packet.getIPv4SourceN(), packet.getIPv4DestN(), 
		 packet.getSourcePort(), packet.getDestPort()))
    {
      /* Problem writing to logs, untoggle the GUI checkmark to
	 show that logging is disabled. */
      emit toggle_log_AllPackets();
    }
#endif
  }

  /* Chat and Login Server Packets, Discard for now */
  if ((packet.getDestPort() == ChatServerPort) ||
      (packet.getSourcePort() == ChatServerPort))
    return;

  if ( ((packet.getDestPort() >= LoginServerMinPort) ||
     (packet.getSourcePort() >= LoginServerMinPort)) &&
     ((packet.getDestPort() <= LoginServerMaxPort) ||
     (packet.getSourcePort() <= LoginServerMaxPort)) )
    return;

  /* discard pure ack/req packets and non valid flags*/
  if (packet.flagsHi() < 0x02 || packet.flagsHi() > 0x46 || size < 10)
  {
#if 0 // ZBTEMP: World Chat server experiments
    //if ((packet.getSourcePort() == WorldServerChatPort) ||
    //	(packet.getDestPort() == WorldServerChatPort))
    {
      printf("discarding packet %s:%d ==>%s:%d flagsHi=%d size=%d\n",
	     (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
	     (const char*)packet.getIPv4DestA(), packet.getDestPort(),
	     packet.flagsHi(), size);
      printf("%s\n", (const char*)packet.headerFlags(false));
      uint32_t crc = packet.calcCRC32();
      if (crc != packet.crc32())
      printf("\tCRC: Warning Packet seq = %d CRC (%08x) != calculated CRC (%08x)!\n",
	     packet.seq(), packet.crc32(), crc);
      else
	printf("\tCRC: Valid CRC seq=%d CRC(%08x) == calculated CRC (%08x)!\n",
	       packet.seq(), packet.crc32(), crc);
    }
#endif
    return;    
  }

#if defined(PACKET_PROCESS_DIAG) && (PACKET_PROCESS_DIAG > 1)
  printf("%s\n", (const char*)packet.headerFlags((PACKET_PROCESS_DIAG < 3)));
  uint32_t crc = packet.calcCRC32();
  if (crc != packet.crc32())
    printf("CRC: Warning Packet seq = %d CRC (%08x) != calculated CRC (%08x)!\n",
	   packet.seq(), packet.crc32(), crc);
#endif

#ifdef PACKET_PEDANTIC
  if (!packet.isValid())
  {
#if defined(PACKET_PROCESS_DIAG) || (PACKET_PEDANTIC > 1)
    printf("INVALID PACKET: Bad CRC32 [%s:%d -> %s:%d] seq %04x len %d crc32 (%08x != %08x)\n",
	   (const char*)packet.getIPv4SourceA(), packet.getSourcePort(),
	   (const char*)packet.getIPv4DestA(), packet.getDestPort(),
	   packet.seq(), 
	   packet.payloadLength(),
	   packet.crc32(), packet.calcCRC32());
#endif
    return;
  }
#endif

  /* Client Detection */
  if (m_detectingClient && packet.getSourcePort() == WorldServerGeneralPort)
  {
      showeq_params->ip = packet.getIPv4DestA();
      m_client_addr = packet.getIPv4DestN();
      m_detectingClient = false;
      emit clientChanged(m_client_addr);
      printf("Client Detected: %s\n", (const char*)showeq_params->ip);
  }
  else if (m_detectingClient && packet.getDestPort() == WorldServerGeneralPort)
  {
     showeq_params->ip = packet.getIPv4SourceA();
     m_client_addr = packet.getIPv4SourceN();
     m_detectingClient = false;
     emit clientChanged(m_client_addr);
     printf("Client Detected: %s\n", (const char*)showeq_params->ip);
  }
  /* end client detection */



  if (packet.getSourcePort() == WorldServerChatPort)
  {
    dispatchWorldChatData(packet.payloadLength(), packet.payload(), DIR_SERVER);

#if 0 // ZBTEMP
    logData ("/tmp/WorldChatData.log", 
	     packet.payloadLength(), 
	     (const uint8_t*)packet.payload(), 
	     packet.getIPv4SourceN(), packet.getIPv4DestN(), 
	     packet.getSourcePort(), packet.getDestPort());
#endif
    return;
  }
  else if (packet.getDestPort() == WorldServerChatPort)
  {
    dispatchWorldChatData(packet.payloadLength(), packet.payload(), DIR_CLIENT);

#if 0 // ZBTEMP
    logData ("/tmp/WorldChatData.log", 
	     packet.payloadLength(), 
	     (const uint8_t*)packet.payload(), 
	     packet.getIPv4SourceN(), packet.getIPv4DestN(), 
	     packet.getSourcePort(), packet.getDestPort());
#endif
    return;
  }


  /* stream identification */
  if (packet.getIPv4SourceN() == m_client_addr)
  {
    if (packet.getDestPort() == WorldServerGeneralPort)
       m_eqstreamid = client2world;
    else 
       m_eqstreamid = client2zone;

    m_eqstreamdir = DIR_CLIENT;
  }
  else if (packet.getIPv4DestN() == m_client_addr)
  {
    if (packet.getSourcePort() == WorldServerGeneralPort)
       m_eqstreamid = world2client;
    else 
       m_eqstreamid = zone2client;
    
    m_eqstreamdir = DIR_SERVER;
  }
  else 
  {
     m_eqstreamid = unknown_stream;
     return;
  } /* end stream identification*/

  emit numPacket(++m_packetCount[m_eqstreamid], (int)m_eqstreamid);
  
  if (!packet.isARQ()) // process packets that don't have an arq sequence
  { 
    // we only handle packets with opcodes
    if (packet.payloadLength() < 2)
      return;

     /* does not require sequencing, so straight to dispatch */
     if ((m_eqstreamid == zone2client) || (m_eqstreamid == client2zone))
     {
        dispatchZoneData (packet.payloadLength(), packet.payload(), m_eqstreamdir);
        return;
     }
     else
     {
       dispatchWorldData (packet.payloadLength(), packet.payload(), m_eqstreamdir);
       return;
     }
  }
  else if (packet.isARQ()) // process ARQ sequences
  {
     uint16_t serverArqSeq = packet.arq();
     emit seqReceive(serverArqSeq, (int)m_eqstreamid);
      
     /* this conditions should only be met once per zone/world, New Sequence */
     if (packet.isSEQStart() && !packet.isClosingLo() && m_session_tracking_enabled < 2)
     {

#ifdef PACKET_PROCESS_DIAG
        printf("EQPacket: SEQStart found, setting arq seq, %04x  stream %s\n",
               serverArqSeq, EQStreamStr[m_eqstreamid]);
#endif
	initDecode();

        // hey, a SEQStart, use it's packet to set ARQ
        m_serverArqSeqExp[m_eqstreamid] = serverArqSeq;
        m_serverArqSeqFound[m_eqstreamid] = true;
        emit seqExpect(m_serverArqSeqExp[m_eqstreamid], (int)m_eqstreamid);


        if (m_eqstreamid == zone2client && m_session_tracking_enabled && !showeq_params->playbackpackets)
        {
           m_session_tracking_enabled = 2;

	   m_serverPort = packet.getSourcePort();
           m_clientPort = packet.getDestPort();
	
	   if (!showeq_params->playbackpackets)
           {
	      if (showeq_params->mac_address.length() == 17)
              {
	         m_packetCapture->setFilter(showeq_params->device, 
  				            showeq_params->mac_address,
				            showeq_params->realtime, 
				            MAC_ADDRESS_TYPE, 0, 
				            m_clientPort);
	         printf ("EQPacket: SEQStart detected, pcap filter: EQ Client %s, Client port %d\n",
		         (const char*)showeq_params->mac_address, 
		         m_clientPort);
	      }
	      else
	      {
	         m_packetCapture->setFilter(showeq_params->device, 
	     			            showeq_params->ip,
				            showeq_params->realtime, 
				            IP_ADDRESS_TYPE, 0, 
				            m_clientPort);
	         printf ("EQPacket: SEQStart detected, pcap filter: EQ Client %s, Client port %d\n",
	  	         (const char*)showeq_params->ip, 
		         m_clientPort);
	      }
	   }
        }

	// notify that the client port has been latched
	emit sessionTrackingChanged(m_session_tracking_enabled);
	emit clientPortLatched(m_clientPort);
     }
     else if (!m_serverArqSeqFound[m_eqstreamid] && m_session_tracking_enabled == 0 &&
              !packet.isClosingHi() && !packet.isClosingLo() && m_eqstreamid == zone2client)
     {
#ifdef PACKET_PROCESS_DIAG
        printf("SEQ: new sequence found, setting arq seq, %04x  stream %d\n", serverArqSeq, m_eqstreamid);
#endif
        m_serverArqSeqExp[m_eqstreamid] = serverArqSeq;
        m_serverArqSeqFound[m_eqstreamid] = true;
        emit seqExpect(m_serverArqSeqExp[m_eqstreamid], (int)m_eqstreamid);
     }

#ifdef PACKET_PROCESS_DIAG
     if (m_eqstreamid == client2zone)
        printf("client2zone diag: seqexp 0x%04x curseq 0x%04x\n", m_serverArqSeqExp[m_eqstreamid], serverArqSeq);
#endif

     // is this the currently expected sequence, if so, do something with it.
     if (m_serverArqSeqExp[m_eqstreamid] == serverArqSeq)
     {
	m_serverArqSeqExp[m_eqstreamid] = serverArqSeq + 1;
	emit seqExpect(m_serverArqSeqExp[m_eqstreamid], (int)m_eqstreamid);

#ifdef PACKET_PROCESS_DIAG
        printf("SEQ: Found next arq in data stream, incrementing arq seq, %04x\n", 
               serverArqSeq);
#endif

        if (!packet.isASQ() && !packet.isFragment() && !packet.isClosingHi())
        {
           // seems to be a sort of ping from client to server, has ARQ
           // but no ASQ, Flags look like 0x0201 (network byte order)
#ifdef PACKET_PROCESS_DIAG
           printf("SEQ: ARQ without ASQ from stream %s arq 0x%04x\n",
                   EQStreamStr[m_eqstreamid], serverArqSeq);
#endif
        }

        // since the servers do not care about client closing sequences, we won't either
        // Hey clients have rights too, or not! 
        else if (packet.isClosingHi() && packet.isClosingLo() && m_eqstreamid == zone2client)
        {
#ifdef PACKET_PROCESS_DIAG
           printf("EQPacket: Closing HI & LO, arq %04x\n", packet.arq());  
#endif
           // reseting the pcap filter to a non-exclusive form allows us to beat 
           // the race condition between timer and processing the zoneServerInfo
           if(!showeq_params->playbackpackets) 
             m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
                                        showeq_params->realtime, IP_ADDRESS_TYPE, 0, 0);
	   printf ("EQPacket: SEQClosing detected, awaiting next zone session,  pcap filter: EQ Client %s\n",
	  	   (const char*)showeq_params->ip);

           if (m_session_tracking_enabled)
              m_session_tracking_enabled = 1; 

           // decode key no longer valid
	   stopDecode();

           // we'll be waiting for a new SEQStart for ALL streams
           // it seems we only ever see a proper closing sequence from the zone server
           // so reset all packet sequence caches 
           resetEQPacket();

           return;

        }
	
        // if the packet is a fragment dispatch to split
        else if (packet.isFragment())
           dispatchSplitData (packet, m_eqstreamdir, m_eqstreamid);

        else if (packet.payloadLength() >= 2) // has to have an opcode
        {
           if ((m_eqstreamid == zone2client) || (m_eqstreamid == client2zone))
           {
              dispatchZoneData (packet.payloadLength(), packet.payload(), m_eqstreamdir);
           }
           else
           {
	      dispatchWorldData (packet.payloadLength(), packet.payload(), m_eqstreamdir);
           }
        }
     }

     // it's a packet from the future, add it to the cache
     else if ( ( (serverArqSeq > m_serverArqSeqExp[m_eqstreamid]) && 
                 (serverArqSeq < (uint32_t(m_serverArqSeqExp[m_eqstreamid] + arqSeqWrapCutoff))) ) || 
               (serverArqSeq < (int32_t(m_serverArqSeqExp[m_eqstreamid]) - arqSeqWrapCutoff)) ) 
     {
#ifdef PACKET_PROCESS_DIAG
	printf("SEQ: out of order arq %04x stream %d, sending to cache, %04d\n", serverArqSeq, m_serverCache[m_eqstreamid]->size(), m_eqstreamid);
#endif
	
          setCache(serverArqSeq, packet);
     }
	
     // if the cache isn't empty, then check for the expected ARQ sequence
     if (!m_serverCache[m_eqstreamid]->empty()) 
        processCache();

     return;
  } /* end ARQ processing */

  return;
  
} /* end dispatchPacket() */

////////////////////////////////////////////////////
// setCache 
// adds current packet to specified cache
// m_eqstreamid must be set by current packet
//
void EQPacket::setCache(uint16_t serverArqSeq, EQUDPIPPacketFormat& packet)
{

   // check if the entry already exists in the cache
   EQPacketMap::iterator it = m_serverCache[m_eqstreamid]->find(serverArqSeq);

   if (it == m_serverCache[m_eqstreamid]->end())
   {
   // entry doesn't exist, so insert an entry into the cache

#ifdef PACKET_PROCESS_DIAG
      printf("SEQ: Insert arq (%04x) stream %d into cache\n", serverArqSeq, m_eqstreamid);
#endif

      m_serverCache[m_eqstreamid]->insert(EQPacketMap::value_type(serverArqSeq, new EQPacketFormat(packet, true)));
      emit cacheSize(m_serverCache[m_eqstreamid]->size(), (int)m_eqstreamid);
   }
   else
   {
     // replacing an existing entry, make sure the new data is valid
     if (packet.isValid())
     {

#ifdef PACKET_PROCESS_DIAG
        printf("SEQ: Update arq (%04x) stream %d in cache\n", serverArqSeq, m_eqstreamid);
#endif

        *it->second = packet;
     }

#ifdef PACKET_PROCESS_DIAG
     else
        printf("SEQ: Not Updating arq (%04x) stream %d into cache, CRC error!\n",
               serverArqSeq, m_eqstreamid);
#endif

   }

#ifdef PACKET_CACHE_DIAG
   if (m_serverCache[m_eqstreamid]->size() > maxServerCacheCount)
      maxServerCacheCount = m_serverCache[m_eqstreamid]->size();
#endif // PACKET_CACHE_DIAG

}


////////////////////////////////////////////////////
// Cache processing
// m_eqstreamid must be set by current packet
// 
void EQPacket::processCache()
{

#if defined(PACKET_CACHE_DIAG)
  printf("SEQ: START checking cache, arq %04x, stream %d , cache count %04d\n",
         m_serverArqSeqExp[m_eqstreamid], m_eqstreamid, m_serverCache[m_eqstreamid]->size());
#endif
  EQPacketMap::iterator it;
  EQPacketMap::iterator eraseIt;
  EQPacketFormat* pf;

  // check if the cache has grown large enough that we should give up
  // on seeing the current serverArqSeqExp
  // this should really only kick in for people with pathetic
  // network cards that missed the packet.
  if (m_serverCache[m_eqstreamid]->size() >= m_serverArqSeqGiveUp)
  {
     // ok, if the expected server arq sequence isn't here yet, give up

     // attempt to find the current expencted arq seq
     it = m_serverCache[m_eqstreamid]->find(m_serverArqSeqExp[m_eqstreamid]);

     // keep trying to find a new serverArqSeqExp if we haven't found a good
     // one yet...
     while(it == m_serverCache[m_eqstreamid]->end())
     {
        printf("SEQ: Giving up on finding arq %04x in cache stream %d, skipping!\n",
               m_serverArqSeqExp[m_eqstreamid], m_eqstreamid);

        // incremente the expected arq sequence number
        m_serverArqSeqExp[m_eqstreamid]++;
        emit seqExpect(m_serverArqSeqExp[m_eqstreamid], (int)m_eqstreamid);

        // attempt to find the new current expencted arq seq
        it = m_serverCache[m_eqstreamid]->find(m_serverArqSeqExp[m_eqstreamid]);
     }
  }
  else
  {
     // haven't given up yet, just try to find the current serverArqSeqExp

     // attempt to find the current expected ARQ seq
     it = m_serverCache[m_eqstreamid]->find(m_serverArqSeqExp[m_eqstreamid]);
  }


  // iterate over cache until we reach the end or run out of
  // immediate followers
  while (it != m_serverCache[m_eqstreamid]->end())
  {
     // get the PacketFormat for the iterator
     pf = it->second;

     // make sure this is the expected packet
     // (we might have incremented to the one after the one returned
     // by find above).
     if (pf->arq() != m_serverArqSeqExp[m_eqstreamid])
        break;

#ifdef PACKET_CACHE_DIAG
     printf("SEQ: found next arq %04x in cache stream %d, cache count %04d\n",
            m_serverArqSeqExp[m_eqstreamid], m_eqstreamid, m_serverCache[m_eqstreamid]->size());
#endif

#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
     // validate the packet against a memory corruption
     if (!pf->isValid())
     {
        // Something's screwed up
        printf("SEQ: INVALID PACKET: Bad CRC32 in packet in cache with arq %04x!\n",
               pf->arq());
     }
#endif


#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
     printf("SEQ: Found next arq in cache, incrementing arq seq, %04x\n", 
            pf->arq());
#endif

     // Duplicate ARQ processing functionality from dispatchPacket,
     // should prolly be its own function processARQ() or some such beast

     if (!pf->isASQ() && !pf->isFragment() && !pf->isClosingHi())
     {
        // seems to be a sort of ping from client to server, has ARQ
        // but no ASQ, Flags look like 0x0201 (network byte order)
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
        printf("SEQ: ARQ without ASQ from stream %s arq 0x%04x\n",
               EQStreamStr[m_eqstreamid], pf->arq());
#endif
     }

     // since the servers do not care about client closing sequences, we won't either
     // Hey clients have rights too, or not! 
     else if (pf->isClosingHi() && pf->isClosingLo() && m_eqstreamid == zone2client)
     {
#if defined (PACKET_CACHE_DIAG) && (PACKET_CACHE_DIAG > 2)
        printf("EQPacket: Closing HI & LO, arq %04x\n", pf->arq());  
#endif
           
        // reseting the pcap filter to a non-exclusive form allows us to beat 
        // the race condition between timer and processing the zoneServerInfo
        if(!showeq_params->playbackpackets) 
          m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
                                     showeq_params->realtime, IP_ADDRESS_TYPE, 0, 0);
	printf ("EQPacket: SEQClosing detected, awaiting next zone session,  pcap filter: EQ Client %s\n",
                (const char*)showeq_params->ip);

        if (m_session_tracking_enabled)
            m_session_tracking_enabled = 1; 

        // we'll be waiting for a new SEQStart for ALL streams
        // it seems we only ever see a proper closing sequence from the zone server
        // so reset all packet sequence caches 
        resetEQPacket();

        break;
     }
	
     // if the packet isn't a fragment dispatch normally, otherwise to split
     else if (pf->isFragment())
     {
        dispatchSplitData (*pf, m_eqstreamdir, m_eqstreamid);
     }

     else
     {
        if ((m_eqstreamid == zone2client) || (m_eqstreamid == client2zone))
        {
           dispatchZoneData (pf->payloadLength(), pf->payload(), m_eqstreamdir);
        }
        else
        {
	   dispatchWorldData (pf->payloadLength(), pf->payload(), m_eqstreamdir);
        }
     }

     eraseIt = it;

     // increment the current position iterator
     it++;

     // erase the packet from the cache
     m_serverCache[m_eqstreamid]->erase(eraseIt);
     emit cacheSize(m_serverCache[m_eqstreamid]->size(), (int)m_eqstreamid);

#ifdef PACKET_CACHE_DIAG
     printf("SEQ: REMOVING arq %04x from cache, cache count %04d\n",
            pf->arq(), m_serverCache[m_eqstreamid]->size());
#endif
     // delete the packet
     delete pf;

     // increment the expected arq number
     m_serverArqSeqExp[m_eqstreamid]++;
     emit seqExpect(m_serverArqSeqExp[m_eqstreamid], (int)m_eqstreamid);

     if (m_serverArqSeqExp[m_eqstreamid] == 0)
        it = m_serverCache[m_eqstreamid]->begin();
  }
  

#ifdef PACKET_CACHE_DIAG
  printf("SEQ: FINISHED checking cache, arq %04x, cache count %04d\n",
         m_serverArqSeqExp[m_eqstreamid], m_serverCache[m_eqstreamid]->size());
#endif
}

////////////////////////////////////////////////////
// cache reset
//
void EQPacket::resetCache(int stream)
{
    // first delete all the entries
    EQPacketMap::iterator it = m_serverCache[stream]->begin();
    EQPacketFormat* pf;
    fprintf(stderr, "Clearing Cache[%s]: Count: %d\n", EQStreamStr[stream], m_serverCache[stream]->size());
    while (it != m_serverCache[stream]->end())
    {
      pf = it->second;
      delete pf;
      it++;
    }

    // now clear the cache
    printf ("Resetting sequence cache[%s]\n", EQStreamStr[stream]);
    m_serverCache[stream]->clear();
    emit cacheSize(0, stream);
}



/* Combines data from split packets */
void EQPacket::dispatchSplitData (EQPacketFormat& pf, uint8_t dir, EQStreamID streamid)
{
#ifdef DEBUG_PACKET
   debug ("dispatchSplitData()");
#endif /* DEBUG_PACKET */

#ifdef PACKET_PROCESS_FRAG_DIAG
   printf("dispatchSplitData(): pf.arq 0x%04x, pf.fragSeq 0x%04x, pf.fragCur 0x%04x, pf.fragTot 0x%04x\n", pf.arq(), pf.fragSeq(), pf.fragCur(), pf.fragTot());
#endif /* PACKET_PROCESS_FRAG_DIAG */

   // fragments with ASQ signify the beginning of a new series
   // warn if previous series is incomplete
   // clear and allocate space for the new series
   if (pf.isASQ())
   {
      if (m_fragmentData[streamid] != NULL)
      {
         if (!pf.fragSeq() == 0) // surpress WARNING for duplicate SEQStart/fragment start (e.g.0x3a)
         {
            printf("dispatchSplitData(): WARNING OpCode 0x%04x will not be processed due to loss\n",
                   eqntohuint16(m_fragmentData[streamid]));
            printf("dispatchSplitData(): recieved new fragment seq 0x%04x before completion of 0x%04x\n",
                   pf.fragSeq(), m_fragmentSeq[streamid]);
         }

	 delete [] m_fragmentData[streamid];
	 m_fragmentData[streamid] = NULL;
      }

      m_fragmentDataAllocSize[streamid] = (pf.fragTot() * pf.payloadLength());
      m_fragmentSeq[streamid] = pf.fragSeq();
      m_fragmentCur[streamid] = pf.fragCur();
      m_fragmentDataSize[streamid] = 0;
      m_fragmentData[streamid] = new uint8_t[m_fragmentDataAllocSize[streamid]]; // should be an over estimate

#ifdef PACKET_PROCESS_FRAG_DIAG
      printf("dispatchSplitData(): Allocating %d bytes for fragmentSeq %d, stream %d, OpCode 0x%04x\n",
             (pf.fragTot() * pf.payloadLength()), pf.fragSeq(), streamid, eqntohuint16(m_fragmentData[streamid]));
#endif
   }
   if (m_fragmentData[streamid] != NULL)
   {
      if (pf.fragSeq() != m_fragmentSeq[streamid] || pf.fragCur() != m_fragmentCur[streamid])
      {
         printf("dispatchSplitData(): WARNING OpCode 0x%04x will not be processed due to loss\n",
             eqntohuint16(m_fragmentData[streamid])); 
         printf("dispatchSplitData(): recieved Out-Of-Order fragment seq 0x%04x (0x%04x) expected 0x%04x\n",
              pf.fragCur(), pf.fragSeq(), m_fragmentCur[streamid]);
         return;
      }
      else if ((m_fragmentDataSize[streamid] + pf.payloadLength()) > m_fragmentDataAllocSize[streamid])
      {
         printf("dispatchSplitData(): WARNING OpCode 0x%04x will not be processed due to under allocation\n",
             eqntohuint16(m_fragmentData[streamid])); 
         printf("\a\aWARNING: FragmentDataSize(%d) > sizeof(fragmentData)(%d)\a\a\n",
	     (m_fragmentDataSize[streamid] + pf.payloadLength()), m_fragmentDataAllocSize[streamid]);
      
         delete [] m_fragmentData[streamid];
         m_fragmentData[streamid] = NULL;

         return;
      }
      else
      {
         memcpy((void*)(m_fragmentData[streamid] + m_fragmentDataSize[streamid]), (void*)pf.payload(), pf.payloadLength());

         m_fragmentDataSize[streamid] += pf.payloadLength();
         m_fragmentCur[streamid] = pf.fragCur()+1;

         /* Check if this is last part of data */
         if (pf.fragCur() == (pf.fragTot() - 1))
         {
#ifdef PACKET_PROCESS_DIAG
            printf("SEQ: seq complete, dispatching (opcode=%04x)\n", 
                   eqntohuint16(m_fragmentData[streamid]));
#endif
            if (streamid == client2zone || streamid == zone2client) 
               dispatchZoneData (m_fragmentDataSize[streamid], m_fragmentData[streamid], dir);
            else
               dispatchWorldData(m_fragmentDataSize[streamid], m_fragmentData[streamid], dir);

            delete [] m_fragmentData[streamid];
            m_fragmentData[streamid] = NULL;
            m_fragmentSeq[streamid] = 0;
            m_fragmentCur[streamid] = 0;
            m_fragmentDataAllocSize[streamid] = 0;
            m_fragmentDataSize[streamid] = 0;
         }
   
         return;
      }
#ifdef PACKET_PROCESS_FRAG_DIAG
   }
   else
   {
     printf("dispatchSplitData(): recieved fragment component (fragSeq 0x%04x, fragCur 0x%04x) before fragment start\n",
	    pf.fragSeq(), pf.fragCur());
#endif
   }
}


///////////////////////////////////////////
//EQPacket::dispatchWorldData  
// note this dispatch gets just the payload
void EQPacket::dispatchWorldData (uint32_t len, uint8_t *data, 
				  uint8_t direction)
{
#ifdef DEBUG_PACKET
  debug ("dispatchWorldData()");
#endif /* DEBUG_PACKET */
  
  // we only process packets with an opcode, so need to be at least 2 bytes 
  // long
  if (len < 2)
    return;

  uint16_t opCode;

  //Logging 
  if (showeq_params->logWorldPackets && showeq_params->logRawPackets)
  {
    opCode = *((uint16_t *)data);  

    if (!logData (showeq_params->WorldLogFilename, len-2, data+2, 
		  direction, opCode, "[Raw]"))
      emit toggle_log_WorldData(); //untoggle the GUI checkmark
  }

  packetList pktlist = decodePacket(data, len, direction);
  if (pktlist.size() == 0)
      return;

  for (unsigned int packetNum = 0; packetNum < pktlist.size(); packetNum++) 
  {
    data = pktlist[packetNum].data;
    len = pktlist[packetNum].len;

    opCode = *((uint16_t *)data);  
    data += 2;
    len -= 2;

    //Logging 
    if (showeq_params->logWorldPackets)
    {
      if (!logData (showeq_params->WorldLogFilename, len, data, 
		    direction, opCode, "[Decoded]"))
	emit toggle_log_WorldData(); //untoggle the GUI checkmark
    }

    bool unk = true;
    
    switch (opCode)
    {
      case OP_GuildList: // old GuildListCode:
      {
	  if (direction != DIR_SERVER)
              break;
	  
	  unk = ! ValidatePayload(OP_GuildList, worldGuildListStruct);
	  if (unk)
	      break;
	  
	  emit worldGuildList((const char*)data, len);
	  
	  break;
      } /* end OP_GuildList */

      case OP_MOTD: // old MOTDCode:
      {
	if (direction != DIR_SERVER)
	  break;
	
	unk = false;
	
	emit worldMOTD((const worldMOTDStruct*)data, len, direction);

	break;
      } 

      default:
      {
          unk = true;
          break;
      }
    }

    if (m_viewUnknownData && unk)
    {
      printUnknown(0, opCode, len, data, direction);
    }

    if (pktlist[packetNum].allocated) 
      delete[] pktlist[packetNum].data;

  }

} // end dispatchWorld

///////////////////////////////////////////
//EQPacket::dispatchWorldChatData  
// note this dispatch gets just the payload
void EQPacket::dispatchWorldChatData (uint32_t len, uint8_t *data, 
				      uint8_t dir)
{
#ifdef DEBUG_PACKET
  debug ("dispatchWorldChatData()");
#endif /* DEBUG_PACKET */
  if (len < 10)
    return;
  
  uint16_t opCode = eqntohuint16(data);

  switch (opCode)
  {
  default:
    printf ("%04x - %d (%s)\n", opCode, len,
	    ((dir == DIR_SERVER) ? 
	     "WorldChatServer --> Client" : "Client --> WorldChatServer"));
  }
}

void EQPacket::printUnknown(unsigned int uiOpCodeIndex, uint16_t opCode, uint32_t len, 
			    uint8_t *data, uint8_t dir) {
     QString tmpStr;
       if (len == 2)
       {
	 tmpStr.sprintf ("%s: %#.04x len %i [%s]", 
			 uiOpCodeIndex > 0 ? 
			 MonitoredOpCodeAliasList[uiOpCodeIndex - 1].ascii() : 
			 "UNKNOWN", opCode, len,
			 dir == DIR_SERVER ? "Server --> Client" : "Client --> Server");
	 printf("\n%s\n\t", (const char*)tmpStr);

	 if ((uiOpCodeIndex > 0) && showeq_params->monitorOpCode_Log)
	 {
	   logMessage(showeq_params->monitorOpCode_Filename, tmpStr);
	   logData(showeq_params->monitorOpCode_Filename, len, data);
	 }
       }
       else
       {
	 tmpStr.sprintf ("%s: %#.04x len %02d [%s] ID:%i", 
			 uiOpCodeIndex > 0 ? 
			 MonitoredOpCodeAliasList[uiOpCodeIndex - 1].ascii() : 
			 "UNKNOWN", opCode, len, 
			 dir == DIR_SERVER ? "Server --> Client" : "Client --> Server", 
			 data[3] * 256 + data[2]);

	 if ((uiOpCodeIndex > 0) && showeq_params->monitorOpCode_Log)
	 {
	   logMessage(showeq_params->monitorOpCode_Filename, tmpStr);
	   logData(showeq_params->monitorOpCode_Filename, len, data);
	 }

	 printf("\n%s\n", (const char*)tmpStr);
	 for (uint32_t a = 0; a < len; a ++)
         {
	   if ((data[a] >= 32) && (data[a] <= 126))
	     printf ("%c", data[a]);
	   else
	     printf (".");
	 }
	 
	 printf ("\n");

	 for (uint32_t a = 0; a < len; a ++)
         {
	   if (data[a] < 32)
	     printf ("%c", data[a] + 95);
	   else if (data[a] > 126)
	     printf ("%c", data[a] - 95);
	   else if (data[a] > 221)
	     printf ("%c", data[a] - 190);
	   else
	     printf ("%c", data[a]);
	 }

         printf("\n");

         
	 for (uint32_t a = 0; a < len; a ++)
         {
	     printf ("%02X", data[a]);
	 }

         printf("\n\n"); /* Adding an extra line break makes it easier
			     for people trying to decode the OpCodes to
			     tell where the raw data ends and the next
			     message begins...  -Andon */
       }
}

void EQPacket::dispatchZoneData (uint32_t len, uint8_t *data, 
				 uint8_t dir)
{
#ifdef DEBUG_PACKET
    debug ("dispatchZoneData()");
#endif /* DEBUG_PACKET */

  // we only process packets with an opcode, so need to be at least 2 bytes 
  // long
  if (len < 2)
    return;

    uint16_t opCode;

    // Raw Logging 
    if (showeq_params->logZonePackets && showeq_params->logRawPackets)
    {
      opCode = *((uint16_t *)data);

      if (!logData(showeq_params->ZoneLogFilename, len-2, data+2, 
		    dir, opCode, "[Raw]"))
	emit toggle_log_ZoneData(); //untoggle the GUI checkmark
    }

    QString  tempStr;

    bool unk = true;

    /* Update EQ Time every 50 packets so we don't overload the CPU */
	    
    if (showeq_params->showEQTime && (m_packetCount[m_eqstreamid] % 50 == 0))
    {
       char timeMessage[30];
       time_t timeCurrent = time(NULL);

       struct timeOfDayStruct eqTime;
       getEQTimeOfDay( timeCurrent, &eqTime);

       sprintf(timeMessage,"EQTime [%02d:%02d %s]",
               (eqTime.hour % 12) == 0 ? 12 : (eqTime.hour % 12),
               (eqTime.minute),
               ((eqTime.hour >= 12 && eqTime.hour << 24) ||
               (eqTime.hour == 24 && eqTime.minute == 0)) ? "pm" : "am");
       emit eqTimeChangedStr(QString (timeMessage));
    }

    packetList pktlist = decodePacket(data, len, dir);
    if (pktlist.size() == 0)
	return;

    for (unsigned int packetNum = 0; packetNum < pktlist.size(); packetNum++) {
	data = pktlist[packetNum].data;
	len = pktlist[packetNum].len;

	opCode = *((uint16_t *)data);  
	data += 2;
	len -= 2;

	//Logging 
	if (showeq_params->logZonePackets)
	  {
	    if (!logData(showeq_params->ZoneLogFilename, len, data, 
			  dir, opCode, "[Decoded]"))
	      emit toggle_log_ZoneData(); //untoggle the GUI checkmark
	  }
	
	switch (opCode)
	{
        case OP_ClientUpdate: // old PlayerPosCode:
        {
#ifdef PACKET_PAYLOAD_SIZE_DIAG
	  if ((len != sizeof(playerSpawnPosStruct)) &&
	      (len != sizeof(playerSelfPosStruct)))
	  {
	    fprintf(stderr, "WARNING: OP_ClientUpdate (%04x) (dataLen: %d != sizeof(playerSpawnPosStruct):%d or sizeof(playerSpawnSelfStruct):%d)\n",
		    OP_ClientUpdate, len, 
		    sizeof(playerSpawnPosStruct), sizeof(playerSelfPosStruct));
	    unk = true;
	  }
	  else
	    unk = false;
#else
	  unk = false;
#endif

	  if (len == sizeof(playerSpawnPosStruct))
	    emit playerUpdate((const playerSpawnPosStruct*)data, len, dir);
	  else if (len == sizeof(playerSelfPosStruct))
	    emit playerUpdate((const playerSelfPosStruct*)data, len, dir);
	  else
	    unk = true;

            break;
        }

        case OP_MobUpdate: // old MobUpdateCode:
        {
            unk = ! ValidatePayload(OP_MobUpdate, spawnPositionUpdate);

            emit updateSpawns((const spawnPositionUpdate *)data, len, dir);

            break;
        }

        case OP_WearChange: // old SpawnUpdateCode:
        {
            unk = ! ValidatePayload(OP_WearChange, SpawnUpdateStruct);
	    SpawnUpdateStruct *su = (SpawnUpdateStruct*)data;
//	    printf("SpawnUpdateCode(id=%d, sub=%d, arg1=%d, arg2=%d)\n", 
//		   su->spawnId, su->subcommand, 
//		   su->arg1, su->arg2);
	    switch(su->subcommand) {
	    case 17:
		emit updateSpawnMaxHP(su, len, dir);
		break;
	    }
            break;
	    emit updateSpawnInfo(su, len, dir);
        }

        case OP_SpawnAppearance: // old SpawnAppearanceCode:
        {
            struct spawnAppearanceStruct * appearance;

            unk = false;

	    emit spawnAppearance((const spawnAppearanceStruct*)data, len, dir);

            appearance = (struct spawnAppearanceStruct *) (data);
            
            switch (appearance->type)
            {
	    case 0x0003:
	    {
		/*
		** Invisibility Flag
		** 0x00 Visible
		** 0x01 Invisible
		*/

#if 0
		if( m_Spawns.contains(appearance->spawnId) )
		    printf("%s ",m_Spawns[appearance->spawnId].name);
		else
		    printf("spawn(%d) ", appearance->spawnId);

		if(appearance->paramter)
		    printf("is now invisible\n");
		else
		    printf("is now visible\n");
#endif 
		break;
	    }

	    case 0x000e:
	    {
		/*
		** Shows how to display the other PCs and NPCs
		** 0x64 Standing Up
		** 0x6e Sitting Down
		*/
 
		break;
	    }

	    case 0x0010:
	    {
		/*
		** This seems to be where the client is
		** assigned the players ID
		*/
		 
	      emit setPlayerID( appearance->paramter );

		break;
	    }

	    case 0x0011:
	    {
		/*
		** This contains the current HP of the player.
		*/
#if 0
		printf("Your hp is now %d.\n", 
		       (unsigned short)appearance->paramter );
#endif
		break;
	    }

	    case 0x0013:
	    {
		/*
		** Not really sure.  Levitation is one of the events 
		** seen by this. 0x02 Levitating
		*/

#if 0
		if ( m_Spawns.contains(appearance->spawnId) )
		    printf("%s ",m_Spawns[appearance->spawnId].name);
		else
		    printf("spawn(%d) ", appearance->spawnId);

		if (appearance->paramter & 0x0002)
		    printf("is now levitating\n");
		else
		    printf("is no longer levitating\n");
#endif
 
		break;
	    }

	    case 0x0015:
	    {
		/*
		** Used for anonymous and roleplaying
		** 0x01 Anonymous
		** 0x02 Role Playing
		*/

#if 0
		if ( m_Spawns.contains(appearance->spawnId) )
		    printf("%s ",m_Spawns[appearance->spawnId].name);
		else
		    printf("spawn(%d) ", appearance->spawnId);

		if (appearance->paramter & 0x0002)
		    printf("is now role playing\n");
		else if(appearance->paramter & 0x0001)
		    printf("is now anonymous\n");
		else
		    printf("is no longer role playing or anonymous\n");
#endif
 
		break;
	    }
                
	    default:
	    {
#if 0
		printf("%0.4x.%0.4x: ",
		       (unsigned short)appearance->type,
		       (unsigned short)appearance->unknown0008);

		if ( m_Spawns.contains(appearance->spawnId) )
		    printf("%s: ",m_Spawns[appearance->spawnId].name);
		else
		    printf("spawn(%d): ", appearance->spawnId);

		printf("%0.8x\n", (unsigned short)appearance->paramter );
#endif
  
		break;
	    }
            }

            break;
        }

	case OP_Death: // old NewCorpseCode:
	{
	    unk = ! ValidatePayload(OP_Death, newCorpseStruct);

	    emit killSpawn((const newCorpseStruct*) data, len, dir);

	    break;
	} /* end CorpseCode */

	case OP_DeleteSpawn: // old DeleteSpawnCode:
	{
	    unk = ! ValidatePayload(OP_DeleteSpawn, deleteSpawnStruct);

	    emit deleteSpawn((const deleteSpawnStruct*)data, len, dir);

	    break;
	}

	case OP_CommonMessage: // old ChannelMessageCode:
	{
	    unk = false;

	    emit channelMessage((const channelMessageStruct*)data, len, dir);

	    break;
	}

	case OP_SimpleMessage: // old SimpleMessageCode:
	{
	    unk = false;

	    emit simpleMessage((const simpleMessageStruct*)data, len, dir);

	    break;
	}

	case OP_ItemLinkResponse: // old ItemInfoCode:
	{
	  unk = false;

	  if (dir == DIR_SERVER)
	    emit itemInfo((const itemInfoStruct*)data, len, dir);
	  else
	    emit itemInfoReq((const itemInfoReqStruct*)data, len, dir);
	}

	case OP_ItemPacket: // old ItemCode:
	{
	  unk = false;

	  if (dir == DIR_SERVER)
	    emit item((const itemPacketStruct*)data, len, dir);
	  
	  break;
	}

	case OP_FormattedMessage: // old FormattedMessageCode:
	{
	    unk = false;

	    emit formattedMessage((const formattedMessageStruct*)data, len, dir);

	    break;
	}

	case OP_NewSpawn: // old NewSpawnCode:
	{
	    unk = ! ValidatePayload(OP_NewSpawn, newSpawnStruct);

	    //logData ("/tmp/newspawn.log", len, data, dir, opCode);
	    emit newSpawn((const newSpawnStruct*)data, len, dir);

	    break;
	}

	case OP_ItemTextFile: // old BookTextCode:
	{
	    unk = false;

	    printf("BOOK: '%s'\n", ((const bookTextStruct *)data)->text);
	    emit bookText((const bookTextStruct*)data, len, dir);

            break;
        }

	case OP_MoneyOnCorpse: // old MoneyOnCorpseCode:
	{
	    unk = ! ValidatePayload(OP_MoneyOnCorpse, moneyOnCorpseStruct);

	    emit moneyOnCorpse((const moneyOnCorpseStruct*)data, len, dir);

	    break;
	} /* end MoneyOnCorpseCode */

        case OP_RandomReply: // old RandomCode:
        {
            unk = ! ValidatePayload(OP_RandomReply, randomStruct);

	    emit random((const randomStruct*)data, len, dir);

            break;
        }

        case OP_RandomReq: // RandomReqCode:
        {
            unk = ! ValidatePayload(OP_RandomReq, randomReqStruct);

	    emit random((const randomReqStruct*)data, len, dir);

            break;
        }

        case OP_Emote: // old EmoteEmoteTextCode:
        {
            unk = false;

	    emit emoteText((const emoteTextStruct*)data, len, dir);

            break;
        }

        case OP_CorpseLocResponse: // old CorpseLocCode:
        {
	    unk = ! ValidatePayload(OP_CorpseLocResponse, corpseLocStruct);
           
	    emit corpseLoc((const corpseLocStruct*)data, len, dir);

            break;
        }

        case OP_InspectAnswer: // old InspectDataCode:
        {
            unk = ! ValidatePayload(OP_InspectAnswer, inspectDataStruct);

            emit inspectData((const inspectDataStruct *)data, len, dir);

            break;
        }
	
	case OP_HPUpdate: // old NpcHpUpdateCode:
	{
	    unk = ! ValidatePayload(OP_HPUpdate, hpNpcUpdateStruct);

	    emit updateNpcHP((const hpNpcUpdateStruct*)data, len, dir);

	    break;
	}

        case SPMesgCode:
        {
            unk = false;

            emit spMessage((const spMesgStruct *)data, len, dir);

            break;
        }

        case OP_MemorizeSpell: // old MemSpellCode:
        {
            unk = ! ValidatePayload(OP_MemorizeSpell, memSpellStruct);

            emit handleSpell((const memSpellStruct*)data, len, dir);

            break;
        }

        case OP_BeginCast: // old BeginCastCode
        {
            unk = ! ValidatePayload(OP_BeginCast, beginCastStruct);

            emit beginCast((const beginCastStruct*)data, len, dir);

            break;
        }

        case OP_CastSpell: // old StartCastCode:
        {
	    unk = ! ValidatePayload(OP_CastSpell, startCastStruct);

	    emit startCast((const startCastStruct*)data, len, dir);

	    break;
        }

	case OP_BuffFadeMsg: // old SpellFadeCode:
	{
	     unk = false;

	     emit spellFaded((const spellFadedStruct*)data, len, dir);

	     break;
	}

        case OP_ExpUpdate: // old ExpUpdateCode:
        {
            unk = ! ValidatePayload(OP_ExpUpdate, expUpdateStruct);

            emit updateExp((const expUpdateStruct*)data, len, dir);

            break;
        }

        case OP_LevelUpdate: // old LevelUpUpdateCode:
        {
            unk = ! ValidatePayload(OP_LevelUpdate, levelUpUpdateStruct);

            emit updateLevel((const levelUpUpdateStruct *)data, len, dir);

            break;
        }

        case OP_SkillUpdate: // old SkillIncCode
        {
            unk = ! ValidatePayload(OP_SkillUpdate, skillIncStruct);

	    emit increaseSkill((const skillIncStruct *)data, len, dir);

            break;
        }

        case OP_MoveDoor: // old DoorOpenCode:
        {
            unk = false;

	    emit doorOpen(data, len, dir);

            break;
        }

        case OP_Illusion: // old IllusionCode:
        {
            unk = false;

	    emit illusion(data, len, dir);

            break;
        }

        case BadCastCode:
        {
            unk = false; //! ValidatePayload(BadCastCode, badCastStruct);

            emit interruptSpellCast((const badCastStruct*)data, len, dir);

            break;
        }

        case SysMsgCode:
        {
            unk = false;

	    emit systemMessage((const sysMsgStruct*)data, len, dir);

            break;
        }

        case OP_ZoneChange: // old ZoneChangeCode:
        {
            unk = ! ValidatePayload(OP_ZoneChange, zoneChangeStruct);

            // in the process of zoning, server hasn't switched yet.

	    emit zoneChange((const zoneChangeStruct*)data, len, dir);
            break;
        }

        case OP_ZoneEntry: // old ZoneEntryCode:
        {
            // We're only interested in the server version

            if (dir == DIR_CLIENT)
	    {
	        unk = ! ValidatePayload(OP_ZoneEntry, ClientZoneEntryStruct);
	        emit zoneEntry((const ClientZoneEntryStruct*)data, len, dir);
	        break;
            }

            unk = ! ValidatePayload(OP_ZoneEntry, ServerZoneEntryStruct);

            emit zoneEntry((const ServerZoneEntryStruct*)data, len, dir);

            break;
        } /* end ZoneEntryCode */

        case OP_NewZone: // old - NewZoneCode:
        {
            unk = ! ValidatePayload(OP_NewZone, newZoneStruct);

	    emit zoneNew((const newZoneStruct*)data, len, dir);

	    if (m_vPacket)
		printf("New Zone at byte: %ld\n", m_vPacket->FilePos());

            break;
        }

	case OP_PlayerProfile:	// Character Profile server to client - old CharProfileCode
	{
	    unk = false;
	   
	    ValidatePayload(OP_PlayerProfile, charProfileStruct);

	    //logData ("/tmp/charprofile.log", len, data, dir, opCode);
	    emit backfillPlayer((const charProfileStruct*)data, len, DIR_SERVER);

	    break;
	}

	case OP_ZoneSpawns: // ZoneSpawnsCode:
	{
	    unk = false; 
	    
	    emit zoneSpawns((const zoneSpawnsStruct*)data, len, dir);
	    //logData ("/tmp/zonespawn.log", len, data, dir, opCode);

	    break;
	}

	case OP_TimeOfDay: // old TimeOfDayCode:
	{
	    struct timeOfDayStruct *tday;

	    unk = ! ValidatePayload(OP_TimeOfDay, timeOfDayStruct);

	    tday = (struct timeOfDayStruct *) data;

	    // adjust tday to compensate for the new time base 
	    // ZBNOTE: Not the best solution, but limits required changes
	    //         for now. Realisticly all EQ Time handling should
	    //         be centralized somewhere else.
	    tday->hour -= 1;
	    
	    // fill in the reference time info
	    m_eqTime.zoneInTime.minute   = tday->minute;
	    m_eqTime.zoneInTime.hour     = tday->hour;
	    m_eqTime.zoneInTime.day      = tday->day;
	    m_eqTime.zoneInTime.month    = tday->month;
	    m_eqTime.zoneInTime.year     = tday->year;

	    m_eqTime.packetReferenceTime = time(NULL);

	    printf( "TIME: %02d:%02d %02d/%02d/%04d\n",
		    tday->hour,
		    tday->minute,
		    tday->month,
		    tday->day,
		    tday->year
		);
	    emit timeOfDay(tday, len, dir);

	    char timeMessage[30];
	    sprintf(timeMessage,"EQTime [%02d:%02d %s]",
		    (tday->hour % 12) == 0 ? 12 : (tday->hour % 12),
		    (tday->minute),
		    ((tday->hour >= 12 && tday->hour << 24) ||
		     (tday->hour == 24 && tday->minute == 0)) ? "pm" : "am");
	    emit eqTimeChangedStr(QString (timeMessage));
	    printf("%s\n", timeMessage);

	    break;
	}

        case WearChangeCode:
        {
            unk = ! ValidatePayload(WearChangeCode, wearChangeStruct);

            emit spawnWearingUpdate ((const wearChangeStruct*)data, len, dir);

            break;
        }

	case OP_Action:
	{
	  unk = ! ValidatePayload(OP_Action, actionStruct);

	  emit action((const actionStruct*)data, len, dir);

	  break;
	}

        case OP_CastBuff: // old ActionCode:
        {
            unk = false;

            emit action2Message ((const action2Struct *)data, len, dir);

            break;
        }

        case CastOnCode:
        {
            unk = false;

	    emit castOn((castOnStruct*)data, len, dir);

            break;
        }

        case ManaDecrementCode:
        {
            unk = ! ValidatePayload(ManaDecrementCode, manaDecrementStruct);

	    emit manaChange((struct manaDecrementStruct *)data, len, dir);

            break;
        }

        case OP_Stamina: /// old StaminaCode:
        {
            unk = ! ValidatePayload(OP_Stamina, staminaStruct);

	    emit updateStamina((const staminaStruct *)data, len, dir);

            break;
        }

        case OP_GroundSpawn: // old MakeDropCode:
        {
#ifdef PACKET_PAYLOAD_SIZE_DIAG
	  if ((len != sizeof(makeDropStruct)) &&
	      (len != 0))
	  {
	    fprintf(stderr, "WARNING: OP_GroundSpawn (%04x) (dataLen: %d != sizeof(makeDropStruct):%d or 0)\n",
		    OP_GroundSpawn, len, 
		    sizeof(makeDropStruct));
	    unk = true;
	  }
	  else
	    unk = false;
#else
	  unk = false;
#endif

	  emit newGroundItem((const makeDropStruct*)data, len, dir);

	  break;
        }

        case OP_ClickObject: // Old RemDropCode:
        {
            unk = ! ValidatePayload(OP_ClickObject, remDropStruct);

            emit removeGroundItem((const remDropStruct *)data, len, dir);

            break;
        }

        case OP_ShopRequest: // old OpenVendorCode:
        {
            unk = false;

	    emit openVendor(data, len, dir);

            break;
        }

        case OP_ShopEnd: // old CloseVendorCode:
        {
            unk = false;

	    emit closeVendor(data, len, dir);

            break;
        }

        case OP_GMTraining: // old OpenGMCode:
        {
            unk = false;

	    emit openGM(data, len, dir);

            break;
        }

        case OP_GMEndTrainingResponse: // old CloseGMCode:
        {
            unk = false;

	    emit closeGM(data, len, dir);

            break;
        }

        case OP_Consider: // old ConsiderCode:
        {
            unk = false;

	    ValidatePayload(OP_Consider, considerStruct);

	    emit consMessage((const considerStruct*)data, len, dir);

            break;
        }

        case OP_TargetMouse: // old ClientTargetCode:
        {
            unk = ! ValidatePayload(OP_TargetMouse, clientTargetStruct);

            emit clientTarget((const clientTargetStruct*) data, len, dir);

            break;
        }

        case OP_SpawnDoor: // old DoorSpawnsCode:
        {
            unk = false;
	    
#ifdef PACKET_PAYLOAD_SIZE_DIAG
            // verify size

            if (len % sizeof(doorStruct) != 0)
            {
	        printf("WARNING: OP_SpawnDoor (%.04x) (dataLen:%d "
                       "%% sizeof(doorStruct):%d) != 0!\n", 
                       OP_SpawnDoor, len, sizeof(doorStruct));

	        unk = true;
		break;
            }
#endif
	    int nDoors = len / sizeof(doorStruct);
	    const DoorSpawnsStruct *doorsStruct = (const DoorSpawnsStruct *)data;
	    for (int i = 0; i < nDoors; i++) {
		emit newDoorSpawn(&doorsStruct->doors[i], len, dir);
	    }
	    emit newDoorSpawns(doorsStruct, len, dir);

            break;
        }

	case OP_SetRunMode: // old cRunToggleCode:
	{
	    //unk = ! ValidatePayload(cRunToggleCode, cRunToggleStruct);
	    //emit cRunToggle((const cRunToggleStruct*)data, len, dir);
	    unk = false;
	    break;
	}

	case OP_Jump: // old cJumpCode:
	{
	    //no data
	    unk = false;
	    break;
	}

	case OP_Camp: // old cStartCampingCode:
	{
	    //no data
	    unk = false;
	    break;
	}

	case OP_SenseHeading: // old cSenseHeadingCode:
	{
	    //no data
	    unk = false;
	    break;
	}

	case OP_Forage: // old ForageCode:
	{
	    //no data
	    unk = false;
	    break;
	}

	case OP_ConsiderCorpse: //unknown contents // old cConCorpseCode:  
	{
	    //unk = ! ValidatePayload(cConCorpseCode, cConCorpseStruct);
	    //emit cConCorpse((const cConCorpseStruct*)data, len, dir);
	    break;
	}

	case OP_Buff: // old BuffDropCode: 
	{
	  unk = ! ValidatePayload(OP_Buff, buffStruct);

	  emit buff((const buffStruct*)data, len, dir);

	  // this is the server 'buff fading' AND the client 'cancel buff'
	  break;
	}

	case OP_LootRequest:  //unknown contents - old cLootCorpseCode
	{
	    //unk = ! ValidatePayload(cLootCorpseCode, cLootCorpseStruct);
	    //emit cLootCorpse((const cLootCorpseStruct*)data, len, dir);
	    break;
	}

	case OP_EndLootRequest:  //unknown contents - old cDoneLootingCode
	{
	    //unk = ! ValidatePayload(cDoneLootingCode, cDoneLootingStruct);
	    //emit cDoneLooting((const cDoneLootingStruct*)data, len, dir);
	    break;
	}

	case OP_LootComplete:  //unknown contents - old sDoneLootingCode
	{
	    //unk = ! ValidatePayload(sDoneLootingCode, sDoneLootingStruct);
	    //emit sDoneLooting((const sDoneLootingStruct*)data, len, dir);
	    break;
	}

	case OP_WhoAllRequest:  //unknown contents - old WhoAllReqCode
	{
	    //unk = ! ValidatePayload(cWhoAllCode, cWhoAllStruct);
	    //emit cWhoAll((const cWhoAllStruct*)data, len, dir);
	    break;
	}

	case OP_WhoAllResponse: // old sWhoAllOutputCode: unknown contents
	{
	    //unk = ! ValidatePayload(sWhoAllOutputCode, sWhoAllOutputStruct);
	    //emit sWhoAllOutput((const sWhoAllOutputStruct*)data, len, dir);
	    break;
	}

	case OP_ShopTakeMoney:  //unknown contents - old BuyItemCode
	{
	    //unk = ! ValidatePayload(xBuyItemCode, xBuyItemStruct);
	    //emit xBuyItem((const xBuyItemStruct*)data, len, dir);
	    //both the client command and the server acknowledgement when buying
	    break;
	}

	case OP_Logout: // no contents
	{
	  unk = false;

	  emit logOut(data, len, dir);

	  break;
	}

        case OP_GuildMemberList: // old GuildMemberListCode
	{
	    unk = false;
   	    break;
	}

        case OP_GuildMemberUpdate: // old GuildMemberUpdateCode:
	{
	    unk = false;
   	    break;
	}

	case OP_SendZonePoints:
	{
#if 0 // ZBTEMP
	  logData("/tmp/zonePoints.log", len, data, dir, opCode);
	  size_t numZonePoints = (len-4) / 24;
	  uint8_t* curPoint;
	  QString tmpStr;
	  tmpStr.sprintf("NumZonePoints: %d (expect %d)", 
			 *(uint32_t*)data, numZonePoints);
	  logMessage("/tmp/zonePoints.log", tmpStr);
	  for (size_t i = 0; i < numZonePoints; i++)
	  {
	    curPoint = data + 4 + (i * 24);
	    tmpStr.sprintf("zonePoint:%d of %d", i+1, numZonePoints);
	    logMessage("/tmp/zonePoints.log", tmpStr);
	    float* fd = (float*)curPoint;
	    tmpStr.sprintf("%f\t%f\t%f\t%f\t%f\t%f",
			   fd[0], fd[1], fd[2], fd[3], fd[4], fd[5]);
	    logMessage("/tmp/zonePoints.log", tmpStr);
	    int32_t* id = (int32_t*)curPoint;
	    tmpStr.sprintf("%d\t%d\t%d\t%d\t%d\t%d",
			   id[0], id[1], id[2], id[3], id[4], id[5]);
	    logMessage("/tmp/zonePoints.log", tmpStr);
	    int16_t* sd = (int16_t*)curPoint;
	    tmpStr.sprintf("%hu %hu\t%hu %hu\t%hu %hu\t%hu %hu\t%hu %hu\t%hu %hu",
			   sd[0], sd[1], sd[2], sd[3], sd[4], sd[5],
			   sd[6], sd[7], sd[8], sd[9], sd[10], sd[11]);
	    logMessage("/tmp/zonePoints.log", tmpStr);
	  }
#endif // ZBTEMP
	  break;
	}

#if 0 // ZBTEMP: Currently dead opcodes
        case AltExpUpdateCode:
        {
            unk = ! ValidatePayload(AltExpUpdateCode, altExpUpdateStruct);

            emit updateAltExp((const altExpUpdateStruct*)data, len, dir);

            break;
        }

        case Attack2Code:
        {
            unk = false;

            emit attack2Hand1 ((const attack2Struct *)data, len, dir);

            break;
        }

        case NewGuildInZoneCode:
        {
            unk = false;

            break;
        }

        case MoneyUpdateCode:
        {  
            unk = false;

	    emit moneyUpdate((const moneyUpdateStruct*)data, len, dir);

            break;
        }

        case MoneyThingCode:
        {
            unk = false;

	    emit moneyThing((const moneyThingStruct*)data, len, dir);

            break;
        }

        case BindWoundCode:
        {
            unk = false;

	    emit bindWound((bindWoundStruct*)data, len, dir);

            break;
        }

        case GroupInfoCode:
        {
            // Too much still unknown.

            unk = ! ValidatePayload(GroupInfoCode, groupMemberStruct);

	    emit groupInfo((const groupMemberStruct*)data, len, dir);

	    break;
        }

        case GroupInviteCode:
        {
            unk = ! ValidatePayload(GroupInviteCode, groupInviteStruct);

	    emit groupInvite((const groupInviteStruct*)data, len, dir);

	    break;
        }

        case GroupDeclineCode:
        {
            unk = ! ValidatePayload(GroupDeclineCode, groupDeclineStruct);

	    emit groupDecline((const groupDeclineStruct*)data, len, dir);

	    break;
        }

        case GroupAcceptCode:
        {
            unk = ! ValidatePayload(GroupAcceptCode, groupAcceptStruct);

	    emit groupAccept((const groupAcceptStruct*)data, len, dir);

	    break;
        }

        case GroupDeleteCode:
        {
            unk = ! ValidatePayload(GroupDeleteCode, groupDeleteStruct);

	    emit groupDelete((const groupDeleteStruct*)data, len, dir);

	    break;
        }

        case CharUpdateCode:
        {
            break;
        }

	case cChatFiltersCode:
	{
	    //unk = ! ValidatePayload(cChatFiltersCode, cChatFiltersStruct);
	    //emit cChatFilters((const cChatFiltersStruct*)data, len, dir);
	    unk = false;
	    break;
	}

	case cOpenSpellBookCode:
	{
	    //unk = ! ValidatePayload(cOpenSpellBookCode, cOpenSpellBookStruct);
	    //emit cOpenSpellBook((const cOpenSpellBookStruct*)data, len, dir);
	    unk = false;
	    break;
	}

	case OP_SwapSpell: // old TradeSpellBookSlotsCode:
	{
	  unk = ! ValidatePayload(OP_SwapSpell, tradeSpellBookSlotsStruct);
	  emit tradeSpellBookSlots((const tradeSpellBookSlotsStruct*)data, len, dir);
	  break;
	}

	case sSpellFizzleRegainCode:  //unknown contents, also comes when you Forage
	{
	    //unk = ! ValidatePayload(sSpellFizzleRegainCode, sSpellFizzleRegainStruct);
	    //emit sSpellFizzleRegain((const sSpellFizzleRegainStruct*)data, len, dir);
	    break;
	}

	case sSpellInterruptedCode:  //unknown contents
	{
	    //unk = ! ValidatePayload(sSpellInterruptedCode, sSpellInterruptedStruct);
	    //emit sSpellInterrupted((const sSpellInterruptedStruct*)data, len, dir);
	    break;
	}

	case cHideCode:
	{
	    //no data
	    unk = false;
	    break;
	}

	case cSneakCode:
	{
	    //no data
	    unk = false;
	    break;
	}

	case cTrackCode:
	{
	    //no data
	    unk = false;
	    break;
	}
#endif // ZBTEMP // Currently dead opcodes

        default:
        {
        }
	} /* end switch(opCode) */

	if (unk)
	  emit unknownOpcode(data, len, dir);

	if (showeq_params->logUnknownZonePackets && unk)
	{
	  if (!logData(showeq_params->UnknownZoneLogFilename, len, data,
		       dir, opCode, QString()))
	    emit toggle_log_UnknownData(); //untoggle the GUI checkmark
	}

	unsigned int uiOpCodeIndex = 0;

	if (showeq_params->monitorOpCode_Usage)
	{
	    unsigned int uiIndex = 0;

	    for (; ((uiIndex < OPCODE_SLOTS) && (uiOpCodeIndex == 0)); uiIndex ++)
	    {
		if (opCode == MonitoredOpCodeList[ uiIndex ][ 0 ])
		{
		    if ((MonitoredOpCodeList[ uiIndex ][ 1 ] == dir) || 
			(MonitoredOpCodeList[ uiIndex ][ 1 ] == 3))
		    {
			if ((!unk && (MonitoredOpCodeList[ uiIndex ][ 2 ] == 1)) 
			    || unk)
			    uiOpCodeIndex = uiIndex + 1;
		    }
		}
	    }
	}

	if ((m_viewUnknownData && unk) || (uiOpCodeIndex > 0))
	{
	    printUnknown(uiOpCodeIndex, opCode, len, data, dir);
	}
	if (pktlist[packetNum].allocated) 
	    delete[] pktlist[packetNum].data;
    }
}

void EQPacket::setViewUnknownData (bool flag)
{
  m_viewUnknownData = flag;
}

int EQPacket::playbackSpeed(void)
{
  return m_vPacket->playbackSpeed();
}

void EQPacket::setPlayback(int speed)
{
  if (m_vPacket)
  {
    m_vPacket->setPlaybackSpeed(speed);

    QString string("");

    if (speed == 0)
      string.sprintf("Playback speed set Fast as possible");

    else if (speed < 0)
       string.sprintf("Playback paused (']' to resume)");

    else
       string.sprintf("Playback speed set to %d", speed);

    emit stsMessage(string, 5000);

    for(int a = 0; a < MAXSTREAMS; a++)
        emit resetPacket(m_packetCount[a], a);  // this resets the packet average

    emit playbackSpeedChanged(speed);
  }
}

void EQPacket::incPlayback(void)
{
  if (m_vPacket)
  {
    int x = m_vPacket->playbackSpeed();

    switch(x)
    {
       // if we were paused go to 1X not full speed
       case -1:
         x = 1;
         break;

       // can't go faster than full speed
       case 0:
         return;
         break;

       case 9:
         x = 0;
         break;

       default:
         x += 1;
         break;
    }

    setPlayback(x);
  }
}

void EQPacket::decPlayback(void)
{
  if (m_vPacket)
  {
    int x = m_vPacket->playbackSpeed();
    switch(x)
    {
       // paused
       case -1:
         return;
         break;

       // slower than 1 is paused
       case 1:
         x = -1;
         break;

       // if we were full speed goto 9
       case 0:
         x = 9;
         break;

       default:
         x -= 1;
         break;
    }

    setPlayback(x);
  }
}

int EQPacket::getEQTimeOfDay( time_t timeConvert, struct timeOfDayStruct *eqTimeOfDay )
{
   /* check to see if we have a reference time to go by. */
   if( m_eqTime.packetReferenceTime == 0 )
      return 0;

   unsigned long diff = timeConvert - m_eqTime.packetReferenceTime;

   /* There are 3 seconds per 1 EQ Minute */
   diff /= 3;

   /* The minutes range from 0 - 59 */
   diff += m_eqTime.zoneInTime.minute;
   eqTimeOfDay->minute = diff % 60;
   diff /= 60;

   // The hours range from 1-24
   // 1 = 1am
   // 2 = 2am
   // ...
   // 23 = 11 pm
   // 24 = 12 am
   //
   // Modify it so that it works from
   // 0-23 for our calculations
   diff += ( m_eqTime.zoneInTime.hour - 1);
   eqTimeOfDay->hour = (diff%24) + 1;
   diff /= 24;

   // The months range from 1-28
   // Modify it so that it works from
   // 0-27 for our calculations
   diff += ( m_eqTime.zoneInTime.day - 1 );
   eqTimeOfDay->day = (diff%28) + 1;
   diff /= 28;

   // The months range from 1-12
   // Modify it so that it works from
   // 0-11 for our calculations
   diff += ( m_eqTime.zoneInTime.month - 1 );
   eqTimeOfDay->month = (diff%12) + 1;
   diff /= 12;

   eqTimeOfDay->year = m_eqTime.zoneInTime.year + diff;

   return 1;
}

char* getTime(char* pchTime)
{
   time_t clock;
   struct tm* current;
   time(&clock);
   current = localtime(&clock);

   sprintf(pchTime, "%02d.%02d.%02d", current->tm_hour, current->tm_min, current->tm_sec);

   return pchTime;
}

void EQPacket::monitorIPClient(const QString& ip)
{
  showeq_params->ip = ip;
  struct in_addr  ia;
  inet_aton (showeq_params->ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  printf("Listening for IP client: %s\n", (const char*)showeq_params->ip);
  if (!showeq_params->playbackpackets)
    m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
			       showeq_params->realtime, 
			       IP_ADDRESS_TYPE, 0, 0);
}

void EQPacket::monitorMACClient(const QString& mac)
{
  showeq_params->mac_address = mac;
  m_detectingClient = true;
  struct in_addr  ia;
  inet_aton (AUTOMATIC_CLIENT_IP, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  printf("Listening for MAC client: %s\n", 
	 (const char*)showeq_params->mac_address);

  if (!showeq_params->playbackpackets)
    m_packetCapture->setFilter(showeq_params->device, showeq_params->ip,
			       showeq_params->realtime, 
			       IP_ADDRESS_TYPE, 0, 0);
}

void EQPacket::monitorNextClient()
{
  m_detectingClient = true;
  showeq_params->ip = AUTOMATIC_CLIENT_IP;
  struct in_addr  ia;
  inet_aton (showeq_params->ip, &ia);
  m_client_addr = ia.s_addr;
  emit clientChanged(m_client_addr);

  resetEQPacket();

  printf("Listening for next client seen. (you must zone for this to work!)\n");

  if (!showeq_params->playbackpackets)
    m_packetCapture->setFilter(showeq_params->device, NULL,
			       showeq_params->realtime, 
			       DEFAULT_ADDRESS_TYPE, 0, 0);
}

void EQPacket::monitorDevice(const QString& dev)
{
  // set the device to use
  showeq_params->device = dev;

  // make sure we aren't playing back packets
  if (showeq_params->playbackpackets)
    return;

  // stop the current packet capture
  m_packetCapture->stop();

  // setup for capture on new device
  if (!showeq_params->ip.isEmpty())
  {
    struct hostent *he;
    struct in_addr  ia;

    /* Substitute "special" IP which is interpreted 
       to set up a different filter for picking up new sessions */
    
    if (showeq_params->ip == "auto")
      inet_aton (AUTOMATIC_CLIENT_IP, &ia);
    else if (inet_aton (showeq_params->ip, &ia) == 0)
    {
      he = gethostbyname(showeq_params->ip);
      if (!he)
      {
	printf ("Invalid address; %s\n", (const char*)showeq_params->ip);
	exit (0);
      }
      memcpy (&ia, he->h_addr_list[0], he->h_length);
    }
    m_client_addr = ia.s_addr;
    showeq_params->ip = inet_ntoa(ia);
    
    if (showeq_params->ip ==  AUTOMATIC_CLIENT_IP)
    {
      m_detectingClient = true;
      printf("Listening for first client seen.\n");
    }
    else
    {
      m_detectingClient = false;
      printf("Listening for client: %s\n",
	     (const char*)showeq_params->ip);
    }
  }
 
  resetEQPacket();

  // restart packet capture
  if (showeq_params->mac_address.length() == 17)
    m_packetCapture->start(showeq_params->device, 
			   showeq_params->mac_address, 
			   showeq_params->realtime, MAC_ADDRESS_TYPE );
  else
    m_packetCapture->start(showeq_params->device, showeq_params->ip, 
			   showeq_params->realtime, IP_ADDRESS_TYPE );
}

void EQPacket::session_tracking()
{
   m_session_tracking_enabled = showeq_params->session_tracking;
   emit sessionTrackingChanged(m_session_tracking_enabled);
}

void EQPacket::setArqSeqGiveUp(int giveUp)
{
  // a sanity check, if the user set it to below 32, they're prolly nuts
  if (giveUp >= 32)
    showeq_params->arqSeqGiveUp = giveUp;

  // a sanity check, if the user set it to below 32, they're prolly nuts
  if (showeq_params->arqSeqGiveUp >= 32)
    m_serverArqSeqGiveUp = showeq_params->arqSeqGiveUp;
  else
    m_serverArqSeqGiveUp = 32;
}

void EQPacket::resetEQPacket()
{
   for (int a = 0; a < MAXSTREAMS; a++)
   {
       if (!m_serverCache[a]->empty()) 
           resetCache(a);

       if (m_fragmentData[a] != NULL);
       {
           delete [] m_fragmentData[a];
           m_fragmentData[a] = NULL;
       }

       m_serverArqSeqExp[a] = 0;
       m_serverArqSeqFound[a] = FALSE;
       m_fragmentDataSize[a] = 0;
       m_fragmentDataAllocSize[a] = 0;
       m_fragmentSeq[a] = 0;
       m_fragmentCur[a] = 0;

       emit seqExpect(m_serverArqSeqExp[a], a);
       emit seqReceive(0, a);
       emit numPacket(0, a);
   }

   m_clientPort = 0;
   m_serverPort = 0;

   m_eqstreamid = unknown_stream;

   m_session_tracking_enabled = showeq_params->session_tracking;
   emit sessionTrackingChanged(m_session_tracking_enabled);
   emit clientPortLatched(m_clientPort);

   stopDecode();
}

const QString EQPacket::pcapFilter()
{
  // make sure we aren't playing back packets
  if (showeq_params->playbackpackets)
    return QString("Playback");

  return m_packetCapture->getFilter();
}


   
//----------------------------------------------------------------------
// PacketCaptureThread
//  start and stop the thread
//  get packets to the processing engine(dispatchPacket)
PacketCaptureThread::PacketCaptureThread()
{


}

PacketCaptureThread::~PacketCaptureThread()
{
}

void PacketCaptureThread::start(const char *device, const char *host, bool realtime, uint8_t address_type)
{

    char ebuf[256]; // pcap error buffer
    char filter_buf[256]; // pcap filter buffer 
    struct bpf_program bpp;
    struct sched_param sp;

   printf ("Initializing Packet Capture Thread: \n");

   // create pcap style filter expressions
   if (address_type == IP_ADDRESS_TYPE)
   {
      if (strcmp(host, AUTOMATIC_CLIENT_IP) == 0)
      {
          printf ("Filtering packets on device %s, searching for EQ client...\n", device);
          sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether proto 0x0800");
      }
      else
      {
          printf ("Filtering packets on device %s, IP host %s\n", device, host);
          sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and host %s and ether proto 0x0800", host);
      }
   }

   else if (address_type == MAC_ADDRESS_TYPE)
   {
      printf ("Filtering packets on device %s, MAC host %s\n", device, host);
      sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether host %s and ether proto 0x0800", host);
   }

   else
   {
      fprintf (stderr, "pcap_error:filter_string: unknown address_type (%d)\n", address_type);
      exit(0);
   }

   /* A word about pcap_open_live() from the docs
   ** to_ms specifies the read timeout in milliseconds.   The
   ** read timeout is used to arrange that the read not necessarily
   ** return immediately when a packet is seen, but that it wait
   ** for  some amount of time to allow more packets to arrive and 
   ** to read multiple packets from the OS kernel in one operation.
   ** Not all  platforms  support  a read timeout; on platforms that
   ** don't, the read timeout is ignored.
   ** 
   ** In Linux 2.4.x with the to_ms set to 0 we get packets immediatly,
   ** and thats what we need in this application, so don't change it!! 
   ** 
   ** a race condition exists between this thread and the main thread 
   ** any artificial delay in getting packets can cause filtering problems
   ** and cause us to miss new stream when the player zones.
   */
   // initialize the pcap object 
   m_pcache_pcap = pcap_open_live((char *) device, BUFSIZ, true, 0, ebuf);
#ifdef __FreeBSD__
   // if we're on FreeBSD, we need to call ioctl on the file descriptor
   // with BIOCIMMEDIATE to get the kernel Berkeley Packet Filter device
   // to return packets to us immediately, rather than holding them in
   // it's internal buffer... if we don't do this, we end up getting 32K
   // worth of packets all at once, at long intervals -- if someone
   // knows a less hacky way of doing this, I'd love to hear about it.
   // the problem here is that libpcap doesn't expose an API to do this
   // in any way
   int fd = *((int*)m_pcache_pcap);
   int temp = 1;
   if ( ioctl( fd, BIOCIMMEDIATE, &temp ) < 0 )
     fprintf( stderr, "PCAP couldn't set immediate mode on BSD\n" );
#endif
   if (!m_pcache_pcap)
   {
     fprintf(stderr, "pcap_error:pcap_open_live(%s): %s\n", device, ebuf);
     if ((getuid() != 0) && (geteuid() != 0))
       fprintf(stderr, "Make sure you are running ShowEQ as root.\n");
     exit(0);
   }

   setuid(getuid()); // give up root access if running suid root

   if (pcap_compile(m_pcache_pcap, &bpp, filter_buf, 1, 0) == -1)
   {
      pcap_perror (m_pcache_pcap, "pcap_error:pcap_compile");
      exit(0);
   }

   if (pcap_setfilter (m_pcache_pcap, &bpp) == -1)
   {
      pcap_perror (m_pcache_pcap, "pcap_error:pcap_setfilter");
      exit(0);
   }

   m_pcache_first = m_pcache_last = NULL;

   pthread_mutex_init (&m_pcache_mutex, NULL);
   pthread_create (&m_tid, NULL, loop, (void*)this);

   if (realtime)
   {
      memset (&sp, 0, sizeof (sp));
      sp.sched_priority = 1;
      if (pthread_setschedparam (m_tid, SCHED_RR, &sp) != 0)
         fprintf (stderr, "Failed to set capture thread realtime.");
   }
}

void PacketCaptureThread::stop()
{
  // close the pcap session
  pcap_close(m_pcache_pcap);
}

void* PacketCaptureThread::loop (void *param)
{
    PacketCaptureThread* myThis = (PacketCaptureThread*)param;
    pcap_loop (myThis->m_pcache_pcap, -1, packetCallBack, (u_char*)param);
    return NULL;
}

void PacketCaptureThread::packetCallBack(u_char * param, 
					 const struct pcap_pkthdr *ph,
					 const u_char *data)
{
    struct packetCache *pc;
    PacketCaptureThread* myThis = (PacketCaptureThread*)param;
    pc = (struct packetCache *) malloc (sizeof (struct packetCache) + ph->len);
    pc->len = ph->len;
    memcpy (pc->data, data, ph->len);
    pc->next = NULL;

    pthread_mutex_lock (&myThis->m_pcache_mutex);

    if (myThis->m_pcache_last)
       myThis->m_pcache_last->next = pc;

    myThis->m_pcache_last = pc;

    if (!myThis->m_pcache_first)
       myThis->m_pcache_first = pc;

    pthread_mutex_unlock (&myThis->m_pcache_mutex);
}

uint16_t PacketCaptureThread::getPacket(unsigned char *buff)
{
    uint16_t ret;
    struct packetCache *pc = NULL;

    pthread_mutex_lock (&m_pcache_mutex);

    ret = 0;

    pc = m_pcache_first;

    if (pc)
    {
        m_pcache_first = pc->next;

        if (!m_pcache_first)
           m_pcache_last = NULL;
    }

    pthread_mutex_unlock (&m_pcache_mutex);

    if (pc)
    {
       ret = pc->len;
       memcpy (buff, pc->data, ret);
       free (pc);
    }

    return ret;
}

void PacketCaptureThread::setFilter (const char *device,
                                     const char *hostname,
                                     bool realtime,
                                     uint8_t address_type,
                                     uint16_t zone_port,
				     uint16_t client_port
                                    )
{
    char filter_buf[256]; // pcap filter buffer 
    struct bpf_program bpp;
    struct sched_param sp;

    /* Listen to World Server or the specified Zone Server */
    if (address_type == IP_ADDRESS_TYPE && client_port)   
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and host %s and ether proto 0x0800", client_port, client_port, hostname);
    else if (address_type == IP_ADDRESS_TYPE && zone_port) 
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and host %s and ether proto 0x0800", zone_port, zone_port, hostname);
    else if (address_type == MAC_ADDRESS_TYPE && client_port)
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and ether host %s and ether proto 0x0800", client_port, client_port, hostname);
    else if (address_type == MAC_ADDRESS_TYPE && zone_port)
        sprintf (filter_buf, "(udp[0:2] = 9000 or udp[2:2] = 9000 or udp[0:2] = 9876 or udp[0:2] = %d or udp[2:2] = %d) and ether host %s and ether proto 0x0800", zone_port, zone_port, hostname);
    else if (hostname != NULL && !client_port && !zone_port)
         sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether proto 0x0800 and host %s", hostname);
    else
    {
         printf ("Filtering packets on device %s, searching for EQ client...\n", device);
         sprintf (filter_buf, "udp[0:2] > 1024 and udp[2:2] > 1024 and ether proto 0x0800");
    }

    if (pcap_compile (m_pcache_pcap, &bpp, filter_buf, 1, 0) == -1)
    {
        printf("%s\n",filter_buf);
	pcap_perror(m_pcache_pcap, "pcap_error:pcap_compile_error");
        exit (0);
    }

    if (pcap_setfilter (m_pcache_pcap, &bpp) == -1)
    {
        pcap_perror(m_pcache_pcap, "pcap_error:pcap_setfilter_error");
        exit (0);
    }

    if (realtime)
    {
       memset (&sp, 0, sizeof (sp));
       sp.sched_priority = 1;
       if (pthread_setschedparam (m_tid, SCHED_RR, &sp) != 0)
           fprintf (stderr, "Failed to set capture thread realtime.");
    }

    m_pcapFilter = filter_buf;
}

const QString PacketCaptureThread::getFilter()
{
  return m_pcapFilter;
}

