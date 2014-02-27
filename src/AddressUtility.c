#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>

char* PrintSocketAddress(const struct sockaddr *address, FILE *stream, int flag) {
  // Test for address and stream
  if (address == NULL || stream == NULL)
    return NULL;

  void *numericAddress; // Pointer to binary address
  // Buffer to contain result (IPv6 sufficient to hold IPv4)
  char addrBuffer[INET6_ADDRSTRLEN];
  in_port_t port; // Port to print
  // Set pointer to address based on address family
  switch (address->sa_family) {
  case AF_INET:
    numericAddress = &((struct sockaddr_in *) address)->sin_addr;
    port = ntohs(((struct sockaddr_in *) address)->sin_port);
    break;
  case AF_INET6:
    numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
    port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
    break;
  default:
    fputs("[unknown type]", stream);    // Unhandled type
    return NULL;
  }
  // Convert binary to printable address
  if (inet_ntop(address->sa_family, numericAddress, addrBuffer,
      sizeof(addrBuffer)) == NULL) {
    fputs("[invalid address]", stream); // Unable to convert
    return NULL;
    }
  else {
    fprintf(stream, "%s", addrBuffer);
    if (port != 0)                // Zero not valid in any socket addr
      fprintf(stream, "-%u", port);
    if(flag) {
      char *addr = (char *)malloc(INET6_ADDRSTRLEN);
      memset(addr,0,INET6_ADDRSTRLEN);
      memcpy(addr,addrBuffer,INET6_ADDRSTRLEN);
      return addr;
    } else {
      return NULL;
    }
  }

}
