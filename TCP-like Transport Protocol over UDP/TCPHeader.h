#ifndef TCPHeader_defined
#define TCPHeader_defined

#include<iostream>
#include<cstring>

class TCPHeader
{
  private:
    unsigned short SeqNum = 0;
    unsigned short AckNum = 0;
    unsigned short WinSize = 0;
    unsigned short Flags = 0;
    unsigned short Header[4] = {0};
  public:
    //set the numbers in the header
    void setSeqNumber(unsigned long num)
    {
      SeqNum = num % 30721;
    }
    void setAckNumber(unsigned long num)
    {
      AckNum = num % 30721;
    }
    void setWinSize(unsigned short num)
    {
      WinSize = num;
    }
    void setFlags(unsigned short num)
    {
      Flags = num;
    }
    //generate the string that contains the header
    void generate(char* str)
    {
      Header[0] = SeqNum;
      Header[1] = AckNum;
      Header[2] = WinSize;
      Header[3] = Flags;
      memcpy (str, Header, 8);
    }
    //parse a string that contains the header
    void consume(char* str)
    {
      memcpy (Header, str, 8);
      SeqNum = Header[0];
      AckNum = Header[1];
      WinSize = Header[2];
      Flags = Header[3];
    }
    //return the numbers in the header
    unsigned short seqNum()
    {
      return SeqNum;
    }
    unsigned short ackNum()
    {
      return AckNum;
    }
    unsigned short winSize()
    {
      return WinSize;
    }
    unsigned short flags()
    {
      return Flags;
    }
    bool SYN()
    {
      return (bool)((Flags>>1) & 1);
    }
    bool FIN()
    {
      return (bool)(Flags & 1);
    }
    bool ACK()
    {
      return (bool)((Flags>>2) & 1);
    }
};










#endif
