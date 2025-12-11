/*
 * microtcp, a lightweight implementation of TCP for teaching,
 * and academic purposes.
 *
 * Copyright (C) 2015-2017  Manolis Surligas <surligas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "microtcp.h"
#include "../utils/crc32.h"

microtcp_sock_t
microtcp_socket (int domain, int type, int protocol)
{
  /*First create a UDP socket*/
  int sockfd; /*file descriptor representing socket*/
  if((sockfd = socket(domain, type, protocol)) == -1){
    perror("SOCKET COULD NOT BE OPENED");
    exit(EXIT_FAILURE);
  }

  /*Initialize socket*/
  microtcp_sock_t microtcp_socket;
  microtcp_socket.sd = sockfd;
  microtcp_socket.state = CLOSED;
  microtcp_socket.init_win_size = MICROTCP_WIN_SIZE;
  microtcp_socket.curr_win_size = MICROTCP_WIN_SIZE;
  microtcp_socket.recvbuf = (uint8_t*) malloc(MICROTCP_RECVBUF_LEN * sizeof(uint8_t));
  microtcp_socket.buff_fill_level = 0;
  microtcp_socket.cwnd = MICROTCP_INIT_CWND;
  microtcp_socket.ssthresh = MICROTCP_INIT_SSTHRESH;
  microtcp_socket.seq_number = rand();
  microtcp_socket.ack_number = 0;
  microtcp_socket.packets_send = 0;
  microtcp_socket.packets_received = 0;
  microtcp_socket.packets_lost = 0;
  microtcp_socket.bytes_send = 0;
  microtcp_socket.bytes_received = 0;
  microtcp_socket.bytes_lost = 0;

  return microtcp_socket;
}

int
microtcp_bind (microtcp_sock_t *socket, const struct sockaddr *address,
               socklen_t address_len)
{
  /* Your code here */
}

int
microtcp_connect (microtcp_sock_t *socket, const struct sockaddr *address,
                  socklen_t address_len)
{
  /*first check if args are valid*/
  if(!socket || !address || address_len <= 0){
    return -1;
  }

  if(socket->state != CLOSED){
    return -1; /*socket must be in CLOSED state*/
  }

  /*create header to send SYN to the server*/
  microtcp_header_t header;
  header.seq_number = socket->seq_number;
  header.ack_number = 0;
  header.control = 0x2;  //SYN bit
  header.window = socket->curr_win_size;
  header.data_len = 0;
  header.future_use0 = 0;
  header.future_use1 = 0;
  header.future_use2 = 0;
  header.checksum = 0;
  header.checksum = crc32((uint8_t*) &header, sizeof(microtcp_header_t));

  /*send SYN to server*/
  if(sendto(socket->sd, &header, sizeof(microtcp_header_t), 0, address, address_len) == -1){ /*0 payload so data len is header len*/
    perror("SYN could not be sent");
    return -1;
  }

  /*wait for SYN-ACK from server(recvfrom blocks the function until it receives data so no need to block ourselves until data arrives)*/
  microtcp_header_t header_received;
  struct sockaddr_in *sender_address;
  socklen_t sender_address_len = sizeof(struct sockaddr_in);
  ssize_t bytes = recvfrom(socket->sd, &header_received, sizeof(microtcp_header_t), 0, &sender_address, &sender_address_len);


  if(bytes == -1){
    perror("SYN-ACK failed");
    return -1;
  }

  /*check if control bits match SYN-ACK*/
  if(header_received.control != 0x12){
    perror("SYN-ACK not received");
    return -1;
  }
  /*update ack seq_num and check for errors in checksum*/
  uint32_t received_checksum = header_received.checksum;
  header_received.checksum = 0;
  uint32_t calculated_checksum = crc32((uint8_t*) &header_received, sizeof(microtcp_header_t));
  if(received_checksum != calculated_checksum){
    perror("Error in checksum of SYN-ACK");
    return -1;
  }

  socket->ack_number = header_received.seq_number + 1;
  socket->seq_number += 1;
  header.seq_number = socket->seq_number;
  header.ack_number = socket->ack_number;

  /*received SYN-ACK so client sends ACK*/
  if(sendto(socket->sd, &header, sizeof(microtcp_header_t), 0, address, address_len) == -1){ /*0 payload so data len is header len*/
    perror("ACK could not be sent");
    return -1;
  }
  socket->state = ESTABLISHED;
  return 0;
}

int
microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,
                 socklen_t address_len)
{
  /* Your code here */
}

int
microtcp_shutdown (microtcp_sock_t *socket, int how)
{
  /* Your code here */
}

ssize_t
microtcp_send (microtcp_sock_t *socket, const void *buffer, size_t length,
               int flags)
{
  /* Your code here */
}

ssize_t
microtcp_recv (microtcp_sock_t *socket, void *buffer, size_t length, int flags)
{
  /* Your code here */
}
