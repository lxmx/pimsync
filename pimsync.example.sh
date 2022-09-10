#!/bin/sh

# DISCLAIMER: This script comes without any warranty.
# It works for me but may not work for you. Back your data up first!

CARD_DIR="/run/media/$USER/palmOne 128"
JPILOT_DIR="$HOME/.jpilot"

ps_dir="$CARD_DIR/PIMSync"

for i in AddressDB DatebookDB MemoDB ToDoDB; do
  pdb_file="$i".pdb
  pc3_file="$i".pc3

  # Check if local changes are present in J-Pilot
  if [ -f "$JPILOT_DIR"/$pc3_file ] && [ -s "$JPILOT_DIR"/$pc3_file ]; then

    # Back up local and card PDBs
    mkdir -p "$JPILOT_DIR"/pimsync_backup_local
    mkdir -p "$JPILOT_DIR"/pimsync_backup_card

    cp "$JPILOT_DIR"/$pdb_file "$JPILOT_DIR"/pimsync_backup_local
    cp "$JPILOT_DIR"/$pc3_file "$JPILOT_DIR"/pimsync_backup_local

    cp "$ps_dir"/$pdb_file "$JPILOT_DIR"/pimsync_backup_card

    # Merge local changes into card versions
    echo -e "\nMerging $pdb_file:"
    jpilot-merge "$ps_dir"/$pdb_file "$JPILOT_DIR"/$pc3_file \
      "$JPILOT_DIR"/$pdb_file
    rm "$JPILOT_DIR"/$pc3_file

    # Overwrite card versions with merged databases
    echo "Overwriting card $pdb_file"
    cp "$JPILOT_DIR"/$pdb_file "$ps_dir"/$pdb_file

  else
    echo "Overwriting local $pdb_file"
    cp "$ps_dir"/$pdb_file "$JPILOT_DIR"/$pdb_file
  fi

done

echo "Done"

