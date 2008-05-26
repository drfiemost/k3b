/* 
 *
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef K3BDATADOC_H
#define K3BDATADOC_H

#include <k3bdoc.h>
#include <k3bdataitem.h>

#include "k3bisooptions.h"

#include <qfileinfo.h>
#include <qstringlist.h>
//Added by qt3to4:

#include <kurl.h>
#include <kio/global.h>
#include "k3b_export.h"

class K3bDataItem;
class K3bRootItem;
class K3bDirItem;
class K3bJob;
class K3bBootItem;
class K3bFileCompilationSizeHandler;

//class K3bView;
class KConfig;
class QString;
class QStringList;
class QDomDocument;
class QDomElement;
class K3bIso9660Directory;

namespace K3bDevice {
    class Device;
}


/**
 *@author Sebastian Trueg
 */

class LIBK3B_EXPORT K3bDataDoc : public K3bDoc
{
    Q_OBJECT

public:
    K3bDataDoc( QObject* parent = 0 );
    virtual ~K3bDataDoc();

    virtual int type() const { return DATA; }
    virtual QString typeString() const;

    virtual QString name() const;

    /**
     * The spported media types based on the project size
     * and settings (example: if writing mode == TAO we force
     * CD media)
     */
    virtual int supportedMediaTypes() const;

    enum MultiSessionMode { 
        /**
         * Let the K3bDataJob decide if to close the CD or not.
         * The decision is based on the state of the inserted media
         * (appendable/closed), the size of the project (will it fill 
         * up the CD?), and the free space on the inserted media.
         */
        AUTO,
        NONE, 
        START, 
        CONTINUE, 
        FINISH
    };

    K3bRootItem* root() const { return m_root; }

    virtual bool newDocument();
    virtual void clear();

    virtual KIO::filesize_t size() const;

    /**
     * This is used for multisession where size() also returnes the imported session's size
     */
    virtual KIO::filesize_t burningSize() const;
    virtual K3b::Msf length() const;
    virtual K3b::Msf burningLength() const;

    /**
     * Simply deletes the item if it is removable (meaning isRemovable() returns true.
     * Be aware that you can remove items simply by deleting them even if isRemovable()
     * returns false.
     */
    void removeItem( K3bDataItem* item );

    /**
     * Simply calls reparent.
     */
    void moveItem( K3bDataItem* item, K3bDirItem* newParent );
    void moveItems( const QList<K3bDataItem*>& itemList, K3bDirItem* newParent );

    K3bDirItem* addEmptyDir( const QString& name, K3bDirItem* parent );
	
    QString treatWhitespace( const QString& );
	
    virtual K3bBurnJob* newBurnJob( K3bJobHandler* hdl, QObject* parent = 0 );
	
    MultiSessionMode multiSessionMode() const { return m_multisessionMode; }
    void setMultiSessionMode( MultiSessionMode mode );

    int dataMode() const { return m_dataMode; }
    void setDataMode( int m ) { m_dataMode = m; }

    void setVerifyData( bool b ) { m_verifyData = b; }
    bool verifyData() const { return m_verifyData; }

    static bool nameAlreadyInDir( const QString&, K3bDirItem* );

    /**
     * Most of the options that map to the mkisofs parameters are grouped
     * together in the K3bIsoOptions class to allow easy saving to and loading
     * from a KConfig object.
     */
    const K3bIsoOptions& isoOptions() const { return m_isoOptions; }
    void setIsoOptions( const K3bIsoOptions& );

    QList<K3bBootItem*> bootImages() { return m_bootImages; }
    K3bDataItem* bootCataloge() { return m_bootCataloge; }

    K3bDirItem* bootImageDir();

    /**
     * Create a boot item and also create a boot cataloge file in case none
     * exists in the project.
     *
     * Calling this method has the same effect like creating a new K3bBootItem 
     * instance manually and then calling createBootCatalogeItem.
     *
     * \return The new boot item on success or 0 in case a file with the same
     *         name already exists.
     */
    K3bBootItem* createBootItem( const QString& filename, K3bDirItem* bootDir = 0 );

    /**
     * Create a new boot catalog item.
     * For now this is not called automatically for internal reasons.
     *
     * Call this if you create boot items manually instead of using createBootItem.
     *
     * The boot catalog is automatically deleted once the last boot item is removed
     * from the doc.
     *
     * \return The new boot catalog item or the old one if it already exists.
     */
    K3bDataItem* createBootCatalogeItem( K3bDirItem* bootDir );

    /**
     * This will prepare the filenames as written to the image.
     * These filenames are saved in K3bDataItem::writtenName
     */
    void prepareFilenames();

    /**
     * Returns true if filenames need to be cut due to the limitations of Joliet.
     *
     * This is only valid after a call to @p prepareFilenames()
     */
    bool needToCutFilenames() const { return m_needToCutFilenames; }

    QList<K3bDataItem*> needToCutFilenameItems() const { return m_needToCutFilenameItems; }

    /**
     * Imports a session into the project. This will create K3bSessionImportItems
     * and properly set the imported session size.
     * Some settings will be adjusted to the imported session (joliet, rr).
     * 
     * Be aware that this method is blocking.
     *
     * \return true if the old session was successfully imported, false if no
     *         session could be found.
     *
     * \see clearImportedSession()
     */
    bool importSession( K3bDevice::Device*, int session );

    /**
     * The session number that has been imported.
     * \return The number of the imported session or 0 if no session information
     * was available (last track imported) or -1 if no session was imported.
     */
    int importedSession() const;

    /**
     * Searches for an item by it's local path.
     *
     * NOT IMPLEMENTED YET!
     *
     * \return The items that correspond to the specified local path.
     */
    QList<K3bDataItem*> findItemByLocalPath( const QString& path ) const;

public Q_SLOTS:
    virtual void addUrls( const KUrl::List& urls );

    /**
     * Add urls syncroneously
     * This method adds files recursively including symlinks, hidden, and system files.
     * If a file already exists the new file's name will be appended a number.
     */
    virtual void addUrlsToDir( const KUrl::List& urls, K3bDirItem* dir );

    void clearImportedSession();

    /**
     * Just a convience method to prevent using setIsoOptions for this
     * often used value.
     */
    void setVolumeID( const QString& );

Q_SIGNALS:
    void aboutToRemoveItem( K3bDataItem* );
    void aboutToAddItem( K3bDirItem* futureParent, K3bDataItem* );
    void itemRemoved( K3bDataItem* );
    void itemAdded( K3bDataItem* );

protected:
    /** reimplemented from K3bDoc */
    virtual bool loadDocumentData( QDomElement* root );
    /** reimplemented from K3bDoc */
    virtual bool saveDocumentData( QDomElement* );

    void saveDocumentDataOptions( QDomElement& optionsElem );
    void saveDocumentDataHeader( QDomElement& headerElem );
    bool loadDocumentDataOptions( QDomElement optionsElem );
    bool loadDocumentDataHeader( QDomElement optionsElem );

    K3bFileCompilationSizeHandler* m_sizeHandler;

    //  K3bFileCompilationSizeHandler* m_oldSessionSizeHandler;
    KIO::filesize_t m_oldSessionSize;

private:
    void prepareFilenamesInDir( K3bDirItem* dir );
    void createSessionImportItems( const K3bIso9660Directory*, K3bDirItem* parent );

    /**
     * used by K3bDirItem to inform about removed items.
     */
    void aboutToRemoveItemFromDir( K3bDirItem* parent, K3bDataItem* removedItem );
    void aboutToAddItemToDir( K3bDirItem* parent, K3bDataItem* addedItem );
    void itemRemovedFromDir( K3bDirItem* parent, K3bDataItem* removedItem );
    void itemAddedToDir( K3bDirItem* parent, K3bDataItem* addedItem );

    /**
     * load recursivly
     */
    bool loadDataItem( QDomElement& e, K3bDirItem* parent );
    /**
     * save recursivly
     */
    void saveDataItem( K3bDataItem* item, QDomDocument* doc, QDomElement* parent );

    void informAboutNotFoundFiles();

    // FIXME: move all the members into a private d-pointer structure

    QStringList m_notFoundFiles;
    QStringList m_noPermissionFiles;

    K3bRootItem* m_root;

    int m_dataMode;

    bool m_verifyData;
		
    K3bIsoOptions m_isoOptions;

    MultiSessionMode m_multisessionMode;
    QList<K3bDataItem*> m_oldSession;
    int m_importedSession;

    // boot cd stuff
    K3bDataItem* m_bootCataloge;
    QList<K3bBootItem*> m_bootImages;

    bool m_bExistingItemsReplaceAll;
    bool m_bExistingItemsIgnoreAll;

    bool m_needToCutFilenames;
    QList<K3bDataItem*> m_needToCutFilenameItems;

    friend class K3bMixedDoc;
    friend class K3bDirItem;
};

#endif
