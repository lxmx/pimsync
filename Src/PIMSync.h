/*
 * PIMSync.h
 *
 * header file for PIMSync
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */

#ifndef PIMSYNC_H_
#define PIMSYNC_H_


/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appFileCreator			'PIMS'
#define appName					"PIMSync"
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

#endif /* PIMSYNC_H_ */

#define NUM_DBS 4
#define DBS_TO_COPY { "MemoDB", "ToDoDB", "DatebookDB", "AddressDB" }

Boolean MainFormHandleEvent(EventType * eventP);
void OpenAboutDialog();

#define ALL_VOLUMES 0xFFFF
typedef void (*Messager)( const Char* s, UInt32 cookie );
static void StatusMessage( const Char* s, UInt32 cookie );
static void CopyFromSDDir( UInt16 volRefNum, Char* dir, Messager Message, UInt32 cookie, Boolean* copiedP, Boolean* missingP );
void CopyToSD( UInt16 vol, Messager Message, UInt32 cookie, Boolean* copiedP, Boolean* missingP );
void handleFromSD();
void handleToSD();
