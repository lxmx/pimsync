#include <PalmOS.h>
#include <VFSMgr.h>

#include "PIMSync.h"
#include "Rsc/PIMSync_Rsc.h"

#define MAX_FILENAME 1024

/*
 * FUNCTION: MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:
 *
 * command
 *     menu item id
 */

static Boolean MainFormDoCommand(UInt16 command)
{
	Boolean handled = false;

	switch (command)
	{
		case OptionsAbout:
		{
			OpenAboutDialog();
			handled = true;
			break;
		}

		case MainFromSDButton:
		{
			handleFromSD();
			handled = true;
			break;
		}

		case MainToSDButton:
		{
			handleToSD();
			handled = true;
			break;
		}

		case MainHelpButton:
		{
			OpenAboutDialog();
			handled = true;
			break;
		}

	}

	return handled;
}

void OpenAboutDialog()
{
	FormType * frmP;

	/* Clear the menu status from the display */
	MenuEraseStatus(0);

	/* Display the About Box. */
	frmP = FrmInitForm (AboutForm);
	FrmDoDialog (frmP);
	FrmDeleteForm (frmP);
}

/*
 * FUNCTION: MainFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "MainForm" of this
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

Boolean MainFormHandleEvent(EventType * eventP)
{
	Boolean handled = false;
	FormType * frmP;

	switch (eventP->eType)
	{
		case menuEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case ctlSelectEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);
			handled = true;
			break;

        case frmUpdateEvent:
			/*
			 * To do any custom drawing here, first call
			 * FrmDrawForm(), then do your drawing, and
			 * then set handled to true.
			 */
			break;

	}

	return handled;
}

static Err copyDBToCard ( UInt16 volRefNum, Char *dbName, Char *modeStr, UInt32 item, UInt32 total ) {
  Err   err;
  Char  message[128];
  Char *path;

  StrPrintF ( message, "%s%s to card...", modeStr, dbName );

  path = (Char *)MemPtrNew ( StrLen("/PIMSync") + StrLen(dbName) + 6 );
  StrCopy ( path, "/PIMSync/");
  StrCat ( path, dbName );
  StrCat ( path, ".pdb");

  err = VFSDirCreate ( volRefNum, "/PIMSync");

  err = VFSExportDatabaseToFile( volRefNum, path, 0,
                                        DmFindDatabase(0, dbName));

  if ( err == vfsErrFileAlreadyExists ) {
      Char fileMsg[ 40 ];
      StrPrintF( fileMsg, "Overwriting %s", dbName);
      StatusMessage(fileMsg, 0);

      VFSFileDelete ( volRefNum, path );
      err = VFSExportDatabaseToFile( volRefNum, path, 0,
                                            DmFindDatabase(0, dbName));
  } else {
    err = 0;
  }

  MemPtrFree(path);
  return err;
}

static void CopyFromSDDir( UInt16 volRefNum, Char* dir, Messager Message, UInt32 cookie, Boolean* copiedP, Boolean* missingP )
{
    UInt32        iterator;
    FileInfoType  info;
    FileRef       dirRef;
    Err           err;
    Char          nameBuf[ MAX_FILENAME ];

    err = VFSFileOpen( volRefNum, dir, vfsModeRead, &dirRef );

    if ( err != errNone ) {
        return;
    }

    info.nameP      = nameBuf;
    info.nameBufLen = MAX_FILENAME;

    iterator = vfsIteratorStart;


    while ( iterator != vfsIteratorStop  ) {
        FileRef  fileRef;
        UInt16   attributes;
        UInt32   creator;
        UInt32   type;
        Char     dbName[ dmDBNameLength ];
        Char     fileName[ MAX_FILENAME ];
        LocalID  id;
        UInt16   card;
        Int16    i;

        err = VFSDirEntryEnumerate( dirRef, &iterator, &info );

        if ( err != errNone ){
            break;
        }

        if ( info.attributes & vfsFileAttrDirectory )
            continue;

        if ( MAX_FILENAME <= StrLen( info.nameP ) + StrLen( dir ) + 2 )
            continue;

        if ( StrLen( info.nameP ) < 3 ||
             ( StrCaselessCompare( info.nameP + StrLen( info.nameP ) - 3, "prc" )
             && StrCaselessCompare( info.nameP + StrLen( info.nameP ) - 3, "pdb" ) ) )
            continue;

        StrCopy( fileName, dir );
        StrCopy( fileName + StrLen( fileName ), "/" );
        StrCopy( fileName + StrLen( fileName ), info.nameP );

        err = VFSFileOpen( volRefNum, fileName, vfsModeRead, &fileRef );

        if ( err != errNone ) {
            if ( missingP != NULL )
                *missingP = true;
            continue;
        }

        err = VFSFileDBInfo( fileRef, dbName, &attributes,
                  NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
                  &type, &creator, NULL );

        if ( err != errNone ) {
            Message("(no name)...error", cookie );
            if ( missingP != NULL )
                *missingP = true;
            continue;
        }

        VFSFileClose( fileRef );

        for ( i = MemNumCards() - 1 ; 0 <= i ; i-- ) {
             id = DmFindDatabase( i, dbName );
             if ( id != NULL ) {

                 if ( errNone != DmDeleteDatabase( i, id ) ) {
                     SysNotifyDBInfoType d;
                     Boolean deleted = false;

                     if ( errNone == DmDatabaseInfo( i, id, d.dbName, &( d.attributes ), NULL, NULL,
                               NULL, NULL, NULL, NULL, NULL, &d.type, &d.creator ) ) {
                         SysNotifyParamType  n;

                         MemSet( &n, sizeof( n ), 0 );
                         n.notifyType  = sysNotifyDeleteProtectedEvent;
                         n.broadcaster = appFileCreator;
                         d.dbID = id;
                         d.cardNo = i;
                         n.notifyDetailsP = &d;

                         SysNotifyBroadcast( &n );

                         if ( errNone == DmDeleteDatabase( i, id ) ) {
                             deleted = true;;
                         }
                     }
                     if ( ! deleted ) {
                         if ( missingP != NULL )
                             *missingP = true;
                         continue;
                     }
                 }
             }
        }

        err = VFSImportDatabaseFromFile(
                  volRefNum,
                  fileName, &card, &id );

        if ( err == errNone ) {
            UInt32 crDate;
            UInt32 modDate;
            UInt16 attr;

            Char fileMsg[30];
            StrPrintF(fileMsg, "Copied %s", fileName);
            StatusMessage(fileMsg, 0);

            err = DmDatabaseInfo( card, id, NULL, &attr, NULL, &crDate,
                      &modDate, NULL, NULL, NULL, NULL, NULL, NULL );

            if ( err == errNone ) {
                if ( ! ( attr & dmHdrAttrBackup ) ) {
                    attr |= dmHdrAttrBackup;

                    DmSetDatabaseInfo( card, id, NULL, &attr, NULL, NULL,
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL );
                }

                if ( crDate == 0 ) {
                    crDate = TimGetSeconds();

                    DmSetDatabaseInfo( card, id, NULL, NULL, NULL, &crDate,
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL );
                }
                if ( modDate == 0 ) {
                    modDate = TimGetSeconds();

                    DmSetDatabaseInfo( card, id, NULL, NULL, NULL, NULL,
                        &modDate, NULL, NULL, NULL, NULL, NULL, NULL );
                }
            }
            else {
                DmDeleteDatabase( card, id );
            }
        }

        if ( err != errNone ) {
            Message("...error", cookie);
            if ( missingP != NULL )
                *missingP = true;
            continue;
        }
        else {
            if ( copiedP != NULL )
                *copiedP = true;
        }

        VFSFileDelete( volRefNum, fileName );
    }
}

static void StatusMessage( const Char* s, UInt32 cookie )
{
    RectangleType display = { { 10, 120 }, { 140, 20 } };
    WinEraseRectangle( &display, 0 );

    Char statusMsg[50];
    StrPrintF(statusMsg, "Status: %s", s);

    WinDrawChars( statusMsg, StrLen( statusMsg ),
        display.topLeft.x, display.topLeft.y + display.extent.y - FntLineHeight() );
}

void CopyToSD( UInt16 vol, Messager Message, UInt32 cookie, Boolean* copiedP, Boolean* missingP )
{
    UInt16 volRefNum;
    UInt32 volIterator;
    Err    err;

    Char *dbsToCopy[NUM_DBS+1] = DBS_TO_COPY;

    if ( missingP != NULL )
        *missingP = false;

    if ( copiedP != NULL )
        *copiedP = false;

    for ( volIterator = vfsIteratorStart ; volIterator != vfsIteratorStop ; ) {
        err = VFSVolumeEnumerate( &volRefNum, &volIterator );
        if ( err == errNone && ( vol == ALL_VOLUMES || vol == volRefNum ) ) {
            for ( UInt32 i = 0; i < NUM_DBS; i++) {
              copyDBToCard ( volRefNum, dbsToCopy[i], "Copying", i, NUM_DBS);
            }
            StatusMessage("Done", 0);
        }
    }
}

void CopyFromSD( UInt16 vol, Messager Message, UInt32 cookie, Boolean* copiedP, Boolean* missingP )
{
    UInt16 volRefNum;
    UInt32 volIterator;
    Err    err;

    if ( missingP != NULL )
        *missingP = false;

    if ( copiedP != NULL )
        *copiedP = false;

    for ( volIterator = vfsIteratorStart ; volIterator != vfsIteratorStop ; ) {
        err = VFSVolumeEnumerate( &volRefNum, &volIterator );
        if ( err == errNone && ( vol == ALL_VOLUMES || vol == volRefNum ) ) {
            Char driveMsg[ 40 ];

            StrPrintF( driveMsg, "Scanning drive %d...", volRefNum );
            Message( driveMsg, cookie );
            CopyFromSDDir( volRefNum, "/PIMSync", Message, cookie, copiedP, missingP );
        }
    }
}

void handleFromSD()
{
    Boolean copied;
    Boolean missing;
    UInt8 selection;

    selection = FrmAlert( DELETE_ALERT );
    if ( selection == 0 ) {
      CopyFromSD( ALL_VOLUMES, StatusMessage, 0, &copied, &missing );
    }
}

void handleToSD()
{
    Boolean copied;
    Boolean missing;
    StatusMessage("Copying the PIM files...", 0);
    CopyToSD( ALL_VOLUMES, StatusMessage, 0, &copied, &missing );
}

