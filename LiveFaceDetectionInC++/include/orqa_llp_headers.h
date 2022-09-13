#ifndef ORQA_LLP_H
#define ORQA_LLP_H

#define ORQA_REF
#define ORQA_IN
#define ORQA_OUT
#define ORQA_REF

#include <ctype.h>    // ETH_ALEN
#include <net/ethernet.h>    // ETH_ALEN
#include <net/if.h>    // ifreq
#include <linux/if_packet.h> // struct sockaddr_ll
#include <arpa/inet.h>
#include "time.h"

//========================================================

#define ORQA_BYTES_TO_BITRATE(bytes) (((double)(8 * bytes)) / 1e6)

//========================================================

// Size of the header part of the packet:
// sender/destination addresses, timestamp, fragmentation info, payload size, etc.
#define ORQA_LLP_PACKET_HEADER_SIZE (sizeof(struct orqa_llp_frame_packet_t))

// Size of the payload for a single packet
#define ORQA_LLP_PACKET_DATA_SIZE ((ETH_FRAME_LEN - ORQA_LLP_PACKET_HEADER_SIZE) * 1)

// Size of the entire packet, includes header and data
#define ORQA_LLP_PACKET_SIZE (ORQA_LLP_PACKET_HEADER_SIZE + ORQA_LLP_PACKET_DATA_SIZE)

//=========================================================

// LLP packet header which contains the ethernet header, the sender MAC address and the destination MAC address.
struct orqa_llp_header_t
{
  struct ether_header eh;
};

// LLP frame header which contains information about the frame, its 
struct orqa_llp_frame_header_t
{
  // ID of the frame.
  uint16_t index;

  // Amount of fragments in entire frame.
  uint16_t fragments;
};

struct orqa_llp_frame_fragment_t
{
  // Information about the frame that the fragment belongs to.
  struct orqa_llp_frame_header_t frame;

  // Index of current fragment to send
  uint16_t index;

  // Size of the fragment's payload in bytes.
  uint32_t size;
};

//========================================================

// LLP frame fragment packet.
// Contains information about the entire frame and the fragment that it is holding.
struct orqa_llp_frame_packet_t
{
  // Basic ethernet (MAC-layer) header.
  // Keep this at the top of this struct.
  struct orqa_llp_header_t header;
  
  // Metadata about the frame fragment that the packet is holding.
  struct orqa_llp_frame_fragment_t data;

  // Timestamp taken at the time of sending by the TX.
  // Only usable by the TX! The RX would have to return this value to the TX for it to be useable.
  struct timespec timestamp;

  // Calculated by the TX when the RX returns a packet timestamp.
  // Sent to the RX to be displayed on the screen.
  double recent_ping;

  uint8_t reserved[8];
};

//========================================================

// Temporarily disabled
#if 0
extern void orqa_llp_fill_drop_ids(
  ORQA_REF struct orqa_llp_send_info_t *const send_info,
  ORQA_IN unsigned int const drop_percent,
  ORQA_IN uint16_t const fragment_count);
#endif

//========================================================

#endif // ORQA_LLP_H