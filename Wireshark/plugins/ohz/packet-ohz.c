/* packet-ohz.c
 * Routines for Songcast OHZ dissection
 * Copyright 2012, Linn Products Ltd
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glib.h>
#include <epan/packet.h>
#include <epan/prefs.h>


#define OHZ_MIN_PACKET_SIZE 	    8
#define PROTO_NAME 					"Songcast OHZ"
#define PROTO_SHORT_NAME 			"OHZ"
#define PROTO_ABBREV  				"ohz"

#define OHZ_SIGNATURE               0x4f687a20

#define OHZ_TYPE_ZONE_QUERY         0
#define OHZ_TYPE_ZONE_URI           1
#define OHZ_TYPE_PRESET_QUERY       2
#define OHZ_TYPE_PRESET_INFO        3

#define OHZ_PORT                    51972

static const value_string ohz_type_vals[] = {
	{ OHZ_TYPE_ZONE_QUERY,    "Zone Query" },
	{ OHZ_TYPE_ZONE_URI,      "Zone URI" },
	{ OHZ_TYPE_PRESET_QUERY,  "Preset Query" },
	{ OHZ_TYPE_PRESET_INFO,   "Preset Info" },
	{ 0, NULL } 
};


/* Initialize the protocol and registered fields */
static int proto_ohz = -1;
static int hf_signature = -1;
static int hf_version = -1;
static int hf_type = -1;
static int hf_length = -1;
static int hf_zid_len = -1;
static int hf_zone_id = -1;
static int hf_zuri_len = -1;
static int hf_zone_uri = -1;
static int hf_preset_no = -1;
static int hf_preset_mdata_len = -1;
static int hf_preset_metadata = -1;

/* Initialize the subtree pointers */
static gint ett_ohz = -1;

/* Code to actually dissect the packets */
static int dissect_ohz(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	/* Set up structures needed to add the protocol subtree and manage it */
	proto_item *ti;
	proto_tree *ohz_tree;
	guint32 signature, zone_id_len, zone_uri_len, preset_meta_len;
	guint8 type;
	gint offset = 0;
	
	/*
	 * First check that this is an OHZ packet.
	 * If not, return 0 to give another dissector a change to dissect it
	*/
	if (pinfo->destport != OHZ_PORT) {
		return 0;
	}
	if (tvb_length(tvb) < OHZ_MIN_PACKET_SIZE) {
		return 0;
	}
	/* Check the OHZ signature */
	signature = tvb_get_ntohl(tvb, offset);
	if (signature != OHZ_SIGNATURE) {
		return 0;
	}
	offset += 5;
	
	type = tvb_get_guint8(tvb, offset++);

	/* Make entries in Protocol column and Info column on summary display */
	col_set_str(pinfo->cinfo, COL_PROTOCOL, PROTO_SHORT_NAME);
	   
	col_clear(pinfo->cinfo, COL_INFO);
	col_set_str(pinfo->cinfo, COL_INFO, val_to_str(type, ohz_type_vals, "Unknown (%u"));
   
	if (tree) {
		/* Create display subtree for the protocol */
		ti = proto_tree_add_item(tree, proto_ohz, tvb, 0, -1, ENC_NA);
		ohz_tree = proto_item_add_subtree(ti, ett_ohz);

		offset = 0;
		
		/* Signature */
		proto_tree_add_uint(ohz_tree, hf_signature, tvb, offset, 4, signature);
		offset += 4;
		/* Version */
		proto_tree_add_item(ohz_tree, hf_version, tvb, offset, 1, ENC_BIG_ENDIAN);
		offset++;
        /* Type */
		proto_tree_add_uint(ohz_tree, hf_type, tvb, offset, 1, type);
		offset++;
		/* Length */
		proto_tree_add_item(ohz_tree, hf_length, tvb, offset, 2, ENC_BIG_ENDIAN);
		offset += 2;

		switch (type) {
		case OHZ_TYPE_ZONE_QUERY:			
			/* Zone ID length */
			zone_id_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohz_tree, hf_zid_len, tvb, offset, 4, zone_id_len);
			offset += 4;
			/* Zone ID */
			proto_tree_add_item(ohz_tree, hf_zone_id, tvb, offset, zone_id_len, ENC_ASCII);
			offset += zone_id_len;
			break;
			
		case OHZ_TYPE_ZONE_URI:			
		    /* Zone ID length */
			zone_id_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohz_tree, hf_zid_len, tvb, offset, 4, zone_id_len);
			offset += 4;
			/* Zone URI length */
			zone_uri_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohz_tree, hf_zuri_len, tvb, offset, 4, zone_uri_len);
			offset += 4;			
			/* Zone ID */			
			proto_tree_add_item(ohz_tree, hf_zone_id, tvb, offset, zone_id_len, ENC_ASCII);
			offset += zone_id_len;
			/* Zone URI */			
			proto_tree_add_item(ohz_tree, hf_zone_uri, tvb, offset, zone_uri_len, ENC_ASCII);
			offset += zone_uri_len;
			break;
			
		case OHZ_TYPE_PRESET_QUERY:
			/* Preset number */
			proto_tree_add_item(ohz_tree, hf_preset_no, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			break;
			
		case OHZ_TYPE_PRESET_INFO:
		    /* Preset number */
			proto_tree_add_item(ohz_tree, hf_preset_no, tvb, offset, 4, ENC_BIG_ENDIAN);
			offset += 4;
			/* Metadata length */
			preset_meta_len = tvb_get_ntohl(tvb, offset);
			proto_tree_add_uint(ohz_tree, hf_preset_mdata_len, tvb, offset, 4, preset_meta_len);
			offset += 4;
			/* Metadata */
			proto_tree_add_item(ohz_tree, hf_preset_metadata, tvb, offset, preset_meta_len, ENC_ASCII);
			offset += preset_meta_len;
			break;
		}

	}

	/* Return the amount of data this dissector was able to dissect */
	return tvb_length(tvb);
}


/* Register the protocol with Wireshark */

/* this format is required because a script is used to build the C function
   that calls all the protocol registration.
*/

void proto_register_ohz(void)
{
	/* Setup list of header fields */
	static hf_register_info hf[] = {
		{ 
		    &hf_signature,
				{ "Signature", 
				  "ohz.sig",
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
				  "ohz.ver",
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
				  "ohz.type",
				  FT_UINT8, 
				  BASE_DEC, 
				  ohz_type_vals, 
				  0x0,
				  "Packet type", 
				  HFILL }
		},
		{
			&hf_length,
				{ "Packet Length", 
				  "ohz.len",
				  FT_UINT16, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Packet length", 
				  HFILL }
		},
		{
			&hf_zid_len,
				{ "Zone ID Length", 
				  "ohz.zid_len",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Zone ID Length", 
				  HFILL }
		},
		{
			&hf_zone_id,
				{ "Zone ID", 
				  "ohz.zone_id",
				  FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
				  "Zone ID", 
				  HFILL }
		},
		{
			&hf_zuri_len,
				{ "URI Length", 
				  "ohz.uri_len",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Zone URI Length", 
				  HFILL }
		},
		{
			&hf_zone_uri,
				{ "URI", 
				  "ohz.uri",
				  FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
				  "Zone URI", 
				  HFILL }
		},
		{
			&hf_preset_no,
				{ "Preset", 
				  "ohz.preset",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Preset Number", 
				  HFILL }
		},
		{
			&hf_preset_mdata_len,
				{ "Metadata Length", 
				  "ohz.meta_len",
				  FT_UINT32, 
				  BASE_DEC, 
				  NULL, 
				  0x0,
				  "Preset metadata length", 
				  HFILL }
		},
		{
			&hf_preset_metadata,
				{ "Metadata", 
				  "ohz.meta",
				  FT_STRING, 
				  BASE_NONE, 
				  NULL, 
				  0x0,
				  "Preset Metadata", 
				  HFILL }
		}
	};

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_ohz
	};

	/* Register the protocol name and description */
	proto_ohz = proto_register_protocol(
									PROTO_NAME,
									PROTO_SHORT_NAME, 
									PROTO_ABBREV
									);

	/* Required function calls to register the header fields and subtrees used */
	proto_register_field_array(proto_ohz, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}


void proto_reg_handoff_ohz(void)
{
	heur_dissector_add("udp", dissect_ohz, proto_ohz);
}

