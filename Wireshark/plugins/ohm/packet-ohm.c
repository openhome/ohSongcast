/* packet-ohm.c
 * Routines for Songcast OHM/OHU dissection
 *
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glib.h>
#include <epan/packet.h>
#include <epan/prefs.h>


#define OHM_MIN_PACKET_SIZE 	    8
#define PROTO_NAME 					"Songcast OHM"
#define PROTO_SHORT_NAME 			"OHM"
#define PROTO_ABBREV  				"ohm"

#define OHM_SIGNATURE               0x4f686d20

#define OHM_TYPE_JOIN               0
#define OHM_TYPE_LISTEN             1
#define OHM_TYPE_LEAVE              2
#define OHM_TYPE_AUDIO              3
#define OHM_TYPE_TRACK              4
#define OHM_TYPE_METATEXT           5
#define OHM_TYPE_SLAVE              6
#define OHM_TYPE_RESEND             7

#define OHM_PORT                    51972

static const value_string ohm_type_vals[] = {
	{ OHM_TYPE_JOIN,		  "Join" },
	{ OHM_TYPE_LISTEN,        "Listen" },
	{ OHM_TYPE_LEAVE,         "Leave" },
	{ OHM_TYPE_AUDIO,         "Audio" },
	{ OHM_TYPE_TRACK,         "Track" },
	{ OHM_TYPE_METATEXT,      "Metatext" },
	{ OHM_TYPE_SLAVE,         "Slave" },
	{ OHM_TYPE_RESEND,        "Resend" },
	{ 0, NULL }
};


/* 
 * Initialize the protocol header fields 
 */
static int proto_ohm = -1;

/* Common header fields */
static int hf_signature = -1;
static int hf_version = -1;
static int hf_type = -1;
static int hf_length = -1;

/* Audio packet fields */
static int hf_audio_hdr_len = -1;
static int hf_audio_flags = -1;
static int hf_audio_flag_halt = -1;
static int hf_audio_flag_ll = -1;
static int hf_audio_flag_ts = -1;
static int hf_audio_sample_count = -1;
static int hf_audio_frame_no = -1;
static int hf_audio_net_ts = -1;
static int hf_audio_media_latency = -1;
static int hf_audio_media_ts = -1;
static int hf_audio_start_sample = -1;
static int hf_audio_total_samples = -1;
static int hf_audio_sample_rate = -1;
static int hf_audio_bit_rate = -1;
static int hf_audio_vol_offset = -1;
static int hf_audio_bit_depth = -1;
static int hf_audio_channels = -1;
static int hf_audio_rsvd = -1;
static int hf_audio_codec_name_len = -1;
static int hf_audio_codec_name = -1;
static int hf_audio_pcm_samples = -1;

/* Track packet fields */
static int hf_track_seq_no = -1;
static int hf_track_uri_len = -1;
static int hf_track_meta_len = -1;
static int hf_track_uri = -1;
static int hf_track_meta = -1;

/* Meta Text packet fields */
static int hf_text_seq_no = -1;
static int hf_text_len = -1;
static int hf_text_text = -1;

/* Slave packet fields */
static int hf_slave_count = -1;

/* Resend packet fields */
static int hf_resend_count = -1;

static const int *audio_flags[] = {
	&hf_audio_flag_halt,
	&hf_audio_flag_ll,
	&hf_audio_flag_ts,
	NULL
};

/* Initialize the subtree pointers */
static gint ett_ohm = -1;
static gint ett_audio_flags = -1;


/* Code to actually dissect the packets */
static int dissect_ohm(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	/* Set up structures needed to add the protocol subtree and manage it */
	proto_item *ti;
	proto_tree *ohm_tree;
	guint32 i, sig, track_uri_len, track_meta_len, meta_text_len, slave_count;
	guint8 type, codec_name_len;
	gint offset = 0;
	gchar *dst_addr;
	
	/*
	 * First do some checks that this is an OHM packet.
	 * If not, return 0 to give another dissector a change to dissect it
	*/
	
	dst_addr = ep_address_to_str(&pinfo->dst);
	if (!strncmp(dst_addr, "239.253", 7) && (pinfo->destport != OHM_PORT)) {
		return 0;
	}

	if (tvb_length(tvb) < OHM_MIN_PACKET_SIZE)
		return 0;

	/* Check the OHM signature */
	sig = tvb_get_ntohl(tvb, offset);
	if (sig != OHM_SIGNATURE) {
		return 0;
	}
	offset += 5;
	
	type = tvb_get_guint8(tvb, offset++);

	/* Make entries in Protocol column and Info column on summary display */
	col_set_str(pinfo->cinfo, COL_PROTOCOL, PROTO_SHORT_NAME);
	   
	col_clear(pinfo->cinfo, COL_INFO);
	col_set_str(pinfo->cinfo, COL_INFO, val_to_str(type, ohm_type_vals, "Unknown (%u"));
   
	if (tree) {
		/* Create display subtree for the protocol */
		ti = proto_tree_add_item(tree, proto_ohm, tvb, 0, -1, ENC_NA);
		ohm_tree = proto_item_add_subtree(ti, ett_ohm);

		offset = 0;
		
		/* Signature */
		proto_tree_add_item(ohm_tree, hf_signature, tvb, offset, 4, ENC_BIG_ENDIAN);
		offset += 4;
		/* Version */
		proto_tree_add_item(ohm_tree, hf_version, tvb, offset, 1, ENC_BIG_ENDIAN);
		offset++;
        /* Type */
		proto_tree_add_item(ohm_tree, hf_type, tvb, offset, 1, ENC_BIG_ENDIAN);
		offset++;
		/* Length */
		proto_tree_add_item(ohm_tree, hf_length, tvb, offset, 2, ENC_BIG_ENDIAN);
		offset += 2;
		
		switch (type) {
		case OHM_TYPE_AUDIO:
		    /* Header Length */
			proto_tree_add_item(ohm_tree, hf_audio_hdr_len, tvb, offset, 1, ENC_BIG_ENDIAN);
			offset++;
			/* Flags */
			proto_tree_add_bitmask(ohm_tree, tvb, offset, hf_audio_flags, ett_audio_flags, audio_flags, ENC_BIG_ENDIAN);
			offset++;
			/* Sample Count */
			proto_tree_add_item(ohm_tree, hf_audio_sample_count, tvb, offset, 2, ENC_BIG_ENDIAN);
			offset += 2;
			/* Frame Number */
			proto_tree_add_item(ohm_tree, hf_audio_frame_no, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Network Timestamp */
			proto_tree_add_item(ohm_tree, hf_audio_net_ts, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Media Latency */
			proto_tree_add_item(ohm_tree, hf_audio_media_latency, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Media Timestamp */
			proto_tree_add_item(ohm_tree, hf_audio_media_ts, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Start Sample */
			proto_tree_add_item(ohm_tree, hf_audio_start_sample, tvb, offset, 8, ENC_BIG_ENDIAN);
			offset += 8;
			/* Total Samples */
			proto_tree_add_item(ohm_tree, hf_audio_total_samples, tvb, offset, 8, ENC_BIG_ENDIAN);
			offset += 8;
			/* Sample Rate */
			proto_tree_add_item(ohm_tree, hf_audio_sample_rate, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Bit Rate */
			proto_tree_add_item(ohm_tree, hf_audio_bit_rate, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Volume Offset */
			proto_tree_add_item(ohm_tree, hf_audio_vol_offset, tvb, offset, 2, ENC_BIG_ENDIAN);
			offset += 2;
			/* Bit Depth */
			proto_tree_add_item(ohm_tree, hf_audio_bit_depth, tvb, offset, 1, ENC_BIG_ENDIAN);
			offset++;
			/* Channels */
			proto_tree_add_item(ohm_tree, hf_audio_channels, tvb, offset, 1, ENC_BIG_ENDIAN);
			offset++;
			/* Reserved */
			proto_tree_add_item(ohm_tree, hf_audio_rsvd, tvb, offset, 1, ENC_BIG_ENDIAN);
			offset++;
			/* Codec Name Length */
			codec_name_len = tvb_get_guint8(tvb, offset);
			proto_tree_add_uint(ohm_tree, hf_audio_codec_name_len, tvb, offset, 1, codec_name_len);
			offset++;
			/* Codec Name */
			proto_tree_add_item(ohm_tree, hf_audio_codec_name, tvb, offset, codec_name_len, ENC_ASCII);
			offset += codec_name_len;
			/* PCM */
			proto_tree_add_item(ohm_tree, hf_audio_pcm_samples, tvb, offset, -1, ENC_NA);
			break;
		
		case OHM_TYPE_TRACK:
			/* Track Sequence Number */
			proto_tree_add_item(ohm_tree, hf_track_seq_no, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Track URI Length */
			track_uri_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohm_tree, hf_track_uri_len, tvb, offset, 4, track_uri_len);
			offset += 4;
			/* Track Medata Length */
			track_meta_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohm_tree, hf_track_meta_len, tvb, offset, 4, track_meta_len);
			offset += 4;
			/* Track URI */
			proto_tree_add_item(ohm_tree, hf_track_uri, tvb, offset, track_uri_len, ENC_ASCII);
			offset += track_uri_len;
			/* Track Metadata */
			proto_tree_add_item(ohm_tree, hf_track_meta, tvb, offset, track_meta_len, ENC_ASCII);
			offset += track_meta_len;
			break;
			
		case OHM_TYPE_METATEXT:		
			/* Meta Text Sequence Number */
			proto_tree_add_item(ohm_tree, hf_text_seq_no, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Meta Text Length */
			meta_text_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohm_tree, hf_text_len, tvb, offset, 4, meta_text_len);
			offset += 4;
			/* Meta Text */
			proto_tree_add_item(ohm_tree, hf_text_text, tvb, offset, meta_text_len, ENC_ASCII);
			offset += meta_text_len;
			break;
		
		case OHM_TYPE_SLAVE:
			/* Slave Count */
			slave_count = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohm_tree, hf_slave_count, tvb, offset, 4, slave_count);
			offset += 4;
			/* Slave Addresses */
			for (i = 0; i < slave_count; i++) {				
				proto_tree_add_text(ohm_tree, tvb, offset, 6, "Slave[%i] = %s:%u", i, tvb_ip_to_str(tvb, offset), tvb_get_ntohs(tvb, offset + 4));
				offset += 6;
			}
			break;
			
		case OHM_TYPE_RESEND:
			/* Frame count */
			proto_tree_add_item(ohm_tree, hf_resend_count, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
		}
	}

	/* Return the amount of data this dissector was able to dissect */
	return tvb_length(tvb);
}


/* Register the protocol with Wireshark */

/* this format is required because a script is used to build the C function
   that calls all the protocol registration.
*/

void proto_register_ohm(void)
{
	/* Setup list of header fields */
	static hf_register_info hf[] = {
		{ 
		    &hf_signature,
				{ "Signature", 
				  "ohm.sig",
			      FT_UINT32, 
				  BASE_HEX, 
				  NULL, 
				  0x0,
			      "Packet signature", 
				  HFILL }
		},
		{
			&hf_version,
				{ "Version", 
				  "ohm.ver",
			      FT_UINT8, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
			      "Protocol version", 
				  HFILL }
		},
		{
			&hf_type,
				{ "Type", 
				  "ohm.type",
				  FT_UINT8, 
				  BASE_DEC, 
				  ohm_type_vals, 
				  0x0,
				  "Packet type", 
				  HFILL }
		},
		{
			&hf_length,
				{ "Packet Length", 
				  "ohm.len",
				  FT_UINT16, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Packet length", 
				  HFILL }
		},
		{
			&hf_audio_hdr_len,
				{ "Audio Header Length", 
				  "ohm.audio_hdr_len",
			      FT_UINT8, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
			      "Audio Header Length", 
				  HFILL }
		},
		{
			&hf_audio_flags,
				{ "Audio Flags", 
				  "ohm.audio_flags",
			      FT_UINT8, 
				  BASE_HEX, 
				  NULL, 
				  0x0,
			      "Audio Flags", 
				  HFILL }
		},
		{
			&hf_audio_flag_halt,
				{ "Halt", 
				  "ohm.audio_flags.halt",
			      FT_BOOLEAN, 
				  8, 
				  NULL, 
				  0x1,
			      NULL, 
				  HFILL }
		},
		{
			&hf_audio_flag_ll,
				{ "Lossless", 
				  "ohm.audio_flags.lossless",
			      FT_BOOLEAN, 
				  8, 
				  NULL, 
				  0x2,
			      NULL, 
				  HFILL }
		},
		{
			&hf_audio_flag_ts,
				{ "Timestamped", 
				  "ohm.audio_flags.timestamped",
			      FT_BOOLEAN, 
				  8, 
				  NULL, 
				  0x4,
			      NULL, 
				  HFILL }
		},
		{
			&hf_audio_sample_count,
				{ "Sample Count", 
				  "ohm.audio_sample_count",
				  FT_UINT16, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Sample Count", 
				  HFILL }
		},
		{
			&hf_audio_frame_no,
				{ "Frame Number", 
				  "ohm.audio_frame_no",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Frame Number", 
				  HFILL }
		},
		{
			&hf_audio_net_ts,
				{ "Network Timestamp", 
				  "ohm.audio_net_ts",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Network Timestamp", 
				  HFILL }
		},
		{
			&hf_audio_media_latency,
				{ "Media Latency", 
				  "ohm.audio_med_lat",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Media Latency", 
				  HFILL }
		},
		{
			&hf_audio_media_ts,
				{ "Media Timestamp", 
				  "ohm.audio_med_ts",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Media Timestamp", 
				  HFILL }
		},
		{
			&hf_audio_start_sample,
				{ "Start Sample", 
				  "ohm.audio_start_smp",
				  FT_UINT64, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Start Sample", 
				  HFILL }
		},
		{
			&hf_audio_total_samples,
				{ "Total Samples", 
				  "ohm.audio_tot_smps",
				  FT_UINT64, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Total Samples", 
				  HFILL }
		},
		{
			&hf_audio_sample_rate,
				{ "Sample Rate", 
				  "ohm.audio_smp_rate",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Sample Rate", 
				  HFILL }
		},
		{
			&hf_audio_bit_rate,
				{ "Bit Rate", 
				  "ohm.audio_bit_rate",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Bit Rate", 
				  HFILL }
		},
		{
			&hf_audio_vol_offset,
				{ "Volume Offset", 
				  "ohm.audio_vol_off",
				  FT_INT16, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Volume Offset", 
				  HFILL }
		},
		{
			&hf_audio_bit_depth,
				{ "Bit Depth", 
				  "ohm.audio_bit_depth",
			      FT_UINT8, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
			      "Bit Depth", 
				  HFILL }
		},
		{
			&hf_audio_channels,
				{ "Channels", 
				  "ohm.audio_chan",
			      FT_UINT8, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
			      "Channels", 
				  HFILL }
		},
		{
			&hf_audio_rsvd,
				{ "Audio Reserved", 
				  "ohm.audio_rsvd",
			      FT_UINT8, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
			      "Audio Reserved", 
				  HFILL }
		},
		{
			&hf_audio_codec_name_len,
				{ "Codec Name Length", 
				  "ohm.audio_codec_name_len",
			      FT_UINT8, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
			      "Codec Name Length", 
				  HFILL }
		},
		{
			&hf_audio_codec_name,
				{ "Codec Name", 
				  "ohm.audio_codec_name",
			      FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
			      "Codec Name", 
				  HFILL }
		},
		{
			&hf_audio_pcm_samples,
				{ "PCM", 
				  "ohm.audio_pcm",
			      FT_BYTES, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
			      "Audio PCM Samples", 
				  HFILL }
		},
		{
			&hf_track_seq_no,
				{ "Sequence Number", 
				  "ohm.track_seq_no",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Track Sequence Number", 
				  HFILL }
		},
		{
			&hf_track_uri_len,
				{ "URI Length", 
				  "ohm.track_uri_len",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Track URI Length", 
				  HFILL }
		},
		{
			&hf_track_meta_len,
				{ "Metadata Length", 
				  "ohm.track_meta_len",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Track Metadata Length", 
				  HFILL }
		},
		{
			&hf_track_uri,
				{ "Track URI", 
				  "ohm.track_uri",
			      FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
			      "Track URI", 
				  HFILL }
		},
		{
			&hf_track_meta,
				{ "Track Metadata", 
				  "ohm.track_meta",
			      FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
			      "Track Metadata", 
				  HFILL }
		},
		{
			&hf_text_seq_no,
				{ "Sequence Number", 
				  "ohm.text_seq_no",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Meta Text Sequence Number", 
				  HFILL }
		},
		{
			&hf_text_len,
				{ "Meta Text Length", 
				  "ohm.text_len",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Track Metadata Length", 
				  HFILL }
		},
		{
			&hf_text_text,
				{ "Meta Text", 
				  "ohm.text_text",
			      FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
			      "Meta Text", 
				  HFILL }
		},
		{
			&hf_slave_count,
				{ "Count", 
				  "ohm.slave_count",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Slave Count", 
				  HFILL }
		},
		{
			&hf_resend_count,
				{ "Frame Count", 
				  "ohm.resend_count",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Frame Count", 
				  HFILL }
		}
	};

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_ohm,
		&ett_audio_flags
	};

	/* Register the protocol name and description */
	proto_ohm = proto_register_protocol(
									PROTO_NAME,
									PROTO_SHORT_NAME, 
									PROTO_ABBREV
									);

	/* Required function calls to register the header fields and subtrees used */
	proto_register_field_array(proto_ohm, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}


void proto_reg_handoff_ohm(void)
{
	heur_dissector_add("udp", dissect_ohm, proto_ohm);
}

