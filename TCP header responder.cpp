/******************************************************************/
//  NAME:        Kate Hamada
//
//  HOMEWORK:    7
//  
//  CLASS:       ICS 212
//
//  INSTRUCTOR:  Ravi Narayan
//
//  DATE:        March 30, 2025
//
//  FILE:        homework7.cpp
//
//  DESCRIPTION:
//   This is the C++ file that can read binary data of a TCP header
//   and generate a response header using C++.
//
/****************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

int readfile(const std::string& filename, unsigned char buffer[]);
int writefile(const std::string& filename, const unsigned char buffer[]);
void printheader(const unsigned char buffer[]);
void makeheader(const unsigned char request[], unsigned char response[]);
void modifyTcpFlags(unsigned char &flags,bool setAckFlag, bool setFinFlag); 


/******************************************************************/
//  Function name: readfile
//
//  DESCRIPTION:   This function reads a binary file and then puts
//                 the bytes into a buffer array.
//
//  Parameters:    filename (const string&) : Name of the file
//                 buffer (unsigned char[]) : An array of binary bytes
//                                  
//
//  Return values:  0 : The file was read successfully
//                 -1 : The file was not able to be read
//
/****************************************************************/ 

int readfile(const std::string& filename, unsigned char buffer[])
{
    size_t size = 20;
    std::fstream file;
    file.open(filename.c_str(), std::ios::in | std::ios::binary);

    // if the file can't be opened
    if (!file.is_open())
    {
        std::cerr << "Error when openning file" << filename << std::endl;
        return -1;
    }
   
    // reads the file 
    file.read(reinterpret_cast<char*>(buffer), size);

    // if the file can't be read
    if (file.fail())
    {
       
        std::cerr << "Error when trying to read the file" << filename << std::endl;
        file.close();
        return -1;
    }

    file.close();
    return 0;
}

/******************************************************************/
//  Function name: writefile
//
//  DESCRIPTION:   This function writes the bytes to a binary file
//                 from an array of bytes.
//
//  Parameters:    filename (const string&) : The file name
//                 buffer (const unsigned char[]) : An array of bytes
//              
//
//  Return values:  0 : The function was able to write to the file
//                 -1 : The function wasn't able to write to the file
//
/****************************************************************/

int writefile(const std::string& filename, const unsigned char buffer[])
{
    size_t size = 20;
    std::fstream file;
    file.open(filename.c_str(), std::ios::out | std::ios::binary);
 
    if (!file.is_open())
    {
        std::cerr << "Error when openning file" << filename << std::endl;
        return -1;
    }
   
    file.write(reinterpret_cast<const char*>(buffer), size);
    
    if (file.fail())
    {
       
        std::cerr << "Error when trying to write to the file" << filename << std::endl;
        file.close();
        return -1;
    }


    file.close();
    return 0;
}

/******************************************************************/
//  Function name: printheader
//
//  DESCRIPTION:   This function prints out the source port,
//                 destination port, sequence number, acknowledgement
//                 number, and the control flags that are active.
//
//  Parameters:    buffer (const unsigned char) : An array of bytes
//                 
//
//  Return values:  N/A
//
/****************************************************************/

void printheader(const unsigned char buffer[])
{ 
   
    // sets it to the first byte and shifts the first byte to the left in order to add the second byte
    unsigned int source = (buffer[1] << 8) | buffer[0];
    unsigned int destination = (buffer[3] << 8) | buffer [2];
    unsigned int sequence = (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
    unsigned int acknowledgement = (buffer[11] << 24) | (buffer[10] << 16) | (buffer[9] << 8) | buffer[8];
    unsigned char control = buffer[13];
    const char* flag_names[] = {"FIN", "SYN", "RST", "PSH", "ACK", "URG"};

    std::cout << "Source port: " << source << std::endl;
    std::cout << "Destination port: " << destination << std::endl;
    std::cout << "Sequence Number: " << sequence << std::endl;
    std::cout << "Acknowledgement Number: " << acknowledgement << std::endl;
    std::cout << "Flags: ";
    for (int i = 5; i >= 0; i--)
    {
        if (control & (1 << i))
        {
            std::cout << flag_names[i] << " ";
        }
    }
    std::cout << std::endl;
}

/******************************************************************/
//  Function name: makeheader
//
//  DESCRIPTION:   This function creates a response header given a 
//                 header. If the source port is greater than 23767
//                 then the source port's 4th and 11th bits are changed.
//                 The sequence port is also incremented by 1. Then the
//                 source and destination port are swaped. The acknowledge
//                 number in the response header is then changed to the 
//                 sequence port. The sequence number in the response head
//                 is then changed to the sequence number of the original.
//
//  Parameters:    request (unsigned char[]) : An array of bytes
//                                             that are going to be
//                                             altered into response. 
//                 response (unsigned char[]) : An array of bytes
//                                              that final altered 
//                                              header.
//
//  Return values:  N/A
//
/****************************************************************/

void makeheader(const unsigned char request[], unsigned char response[])
{
    unsigned int source = (request[1] << 8) | request[0];
    unsigned int sequence = (request[7] << 24) | (request[6] << 16) | (request[5] << 8) | request[4];
    sequence++; 

    // if the source port is > 32767
    if (source & 32768)
    {
       source = source ^ 0x1020;
    }
   
    // switches source with destination port
    response[0] = request[2];
    response[1] = request[3];
    response[3] = (source >> 8) & 0xFF; // the 0xFF makes sure that it only gets the 1 byte from the 2 bye source
    response[2] = source & 0xFF;

    // set's sequence number in response to sequence+1 from request
    response[4] = (sequence) & 0xFF;
    response[5] = (sequence >> 8) & 0xFF;
    response[6] = (sequence >> 16) & 0xFF;
    response[7] = (sequence >> 24) & 0xFF;

    // set's acknowledge number in response to the sequence number from request
    response[8] = request[4];
    response[9] = request[5];
    response[10] = request[6];
    response[11] = request[7];
       
    // if SYN is 1 then it set SYN and ACK bits to 1
    if (request[13] & (1 << 1))
    {
        response[13] = request[13] | 0x12;
    }
    else
    {
        response[13] = request[13];
    } 
    
    // copies over the rest of the bytes
    for (int i=14; i < 20; i++)
    {
        response[i] = request[i]; 
    } 
}

/******************************************************************/
//  Function name: modifyTcpFlags
//
//  DESCRIPTION:   This function alters the bytes from the flags
//                 section of the binary file and then sets the ACK
//                 and the SYN flags to off or on depending on whether
//                 the set variables are true or false.
//
//  Parameters:    flags (unsigned char &) : The reference of the bytes
//                                           of the flags.
//                 setAckFlag (bool) : Determines if ACK will be 
//                                     1 (true) or 0 (false)
//                 setSynFlag (bool) : Determine if SYN will be 
//                                     1 (true) or 0 (false)
//
//  Return values:  N/A
//
/****************************************************************/


void modifyTcpFlags(unsigned char &flags, bool setAckFlag, bool setSynFlag)
{
    if (setAckFlag)
    {
        flags = flags | 0x20;
    } 
    else
    {
        flags = flags & ~0x20;
    }

    if (setSynFlag)
    {
        flags = flags | 0x10;
    }
    else
    {
        flags = flags & ~0x10;
    }
}
   
/******************************************************************/
//  Function name: main
//
//  DESCRIPTION:   This function runs the header functions in order
//                 to show that they function like expected
//
//  Parameters:    argc (int) : The number of elements in argv
//                 argv (char*[]) : An array of arguments passed
//                                  to the program.
//
//  Return values:  0 : The function ended
//
/****************************************************************/
 
int main()
{
    std::string file_name = "request1.bin"; 
    const std::string test = "test.bin"; // empty file to write to    
    unsigned char buffer[20];
    unsigned char respond[20];
    
    // testing readfile
    if (readfile(file_name, buffer) == 0)
    {
        std::cout << "***'request1.bin' file***" << std::endl;
        printheader(buffer);
    }

    makeheader(buffer, respond);
    std::cout << "\nResponse Header: " << std::endl;
    printheader(respond);

    // testing writefile
    /*if (writefile(test, buffer) == 0)
    {
        std::cout << "\n***File written to successfully***" << std::endl;
        readfile(test, buffer);
        printheader(buffer);    
    }*/

    file_name = "request2.bin";
    if (readfile(file_name, buffer) == 0)
    {
        std::cout << "\n***'request2.bin' file***" << std::endl;
        printheader(buffer);
        makeheader(buffer, respond);
        std::cout << "\nResponse Header: " << std::endl;
        printheader(respond);
    }

    file_name = "request3.bin";
    if (readfile(file_name, buffer) == 0)
    {
        std::cout << "\n***'request3.bin' file***" << std::endl;
        printheader(buffer);
        makeheader(buffer, respond);
        std::cout << "\nResponse Header: " << std::endl;
        printheader(respond);
    }
  
    unsigned char flags = buffer[13];
    std::cout << "\n***Before modify function: " << std::endl;
    printheader(buffer);
    modifyTcpFlags(flags, true, true);
    std::cout << "**After modify function with T and T" << std::endl;
    buffer[13] = flags;
    printheader(buffer);
      
    return 0;
}
