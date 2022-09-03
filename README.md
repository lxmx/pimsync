# PIMSync

This simple Palm OS app allows you to export the built-in Memo Pad, Date Book, Address and To Do List databases to the memory card at the `/PIMSync` location - or import them back, replacing the RAM databases.

## Workflow

The above makes sense if you are using J-Pilot on your desktop, and don't want to use a cable to sync your device.

J-Pilot operates on the very same `MemoDB.pdb`, `ToDoDB.pdb`, `DatebookDB.pdb`, `AddressDB.pdb` files you have on your Palm. Which means you can edit the records, then save them back to the device. 

## Thanks

* Alexander Pruss for [MyInstall](https://www.pruss.mobi/dl/) with its VFS code.
* Otávio P. ([Tavisco](https://github.com/Tavisco)) for the UI code example in his Palm Portfolio app.
* [FileCaddy](http://filecaddy.sourceforge.net/) authors for more VFS code.
