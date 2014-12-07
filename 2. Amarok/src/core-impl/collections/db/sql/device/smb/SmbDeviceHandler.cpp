/****************************************************************************************
 * Copyright (c) 2006-2007 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
 * Copyright (c) 2011 Peter C. Ndikuwera <pndiku@gmail.com>                             *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#define DEBUG_PREFIX "SmbDeviceHandler"
 
#include "SmbDeviceHandler.h"

#include "core/support/Debug.h"
#include "core/collections/support/SqlStorage.h"

#include <KUrl>
#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Solid/NetworkShare>

SmbDeviceHandler::SmbDeviceHandler( int deviceId, const QString &server, const QString &share, const QString &mountPoint, const QString &udi )
    : DeviceHandler()
    , m_deviceID( deviceId )
    , m_server( server )
    , m_share( share )
    , m_mountPoint( mountPoint )
    , m_udi( udi )
{
  DEBUG_BLOCK
}

SmbDeviceHandler::SmbDeviceHandler( int deviceId, const QString &mountPoint, const QString &udi )
    : DeviceHandler()
    , m_deviceID( deviceId )
    , m_mountPoint( mountPoint )
    , m_udi( udi )
{
  DEBUG_BLOCK
}

SmbDeviceHandler::~SmbDeviceHandler()
{
}

bool
SmbDeviceHandler::isAvailable() const
{
    return true;
}


QString
SmbDeviceHandler::type() const
{
    return "smb";
}

int
SmbDeviceHandler::getDeviceID()
{
    return m_deviceID;
}

const QString &SmbDeviceHandler::getDevicePath() const
{
    return m_mountPoint;
}

void SmbDeviceHandler::getURL( KUrl &absolutePath, const KUrl &relativePath )
{
    absolutePath.setPath( m_mountPoint );
    absolutePath.addPath( relativePath.path() );
    absolutePath.cleanPath();
}

void SmbDeviceHandler::getPlayableURL( KUrl &absolutePath, const KUrl &relativePath )
{
    getURL( absolutePath, relativePath );
}

bool SmbDeviceHandler::deviceMatchesUdi( const QString &udi ) const
{
  return m_udi == udi;
}

///////////////////////////////////////////////////////////////////////////////
// class SmbDeviceHandlerFactory
///////////////////////////////////////////////////////////////////////////////

QString SmbDeviceHandlerFactory::type( ) const
{
    return "smb";
}

bool SmbDeviceHandlerFactory::canCreateFromMedium( ) const
{
    return true;
}

bool SmbDeviceHandlerFactory::canCreateFromConfig( ) const
{
    return false;
}

bool SmbDeviceHandlerFactory::canHandle( const Solid::Device &device ) const
{
    const Solid::NetworkShare *share = device.as<Solid::NetworkShare>();
    if( !share )
    {
        debug() << __PRETTY_FUNCTION__ << device.udi() << "has no NetworkShare interface";
        return false;
    }
    if( share->type() != Solid::NetworkShare::Cifs )
    {
        debug() << __PRETTY_FUNCTION__ << device.udi() << "has type" << share->type()
                << "but smbfs/cifs type is" << Solid::NetworkShare::Cifs;
        return false;
    }
    const Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
    if( !access )
    {
        debug() << __PRETTY_FUNCTION__ << device.udi() << "has no StorageAccess interface";
        return false;
    }
    if( !access->isAccessible() || access->filePath().isEmpty() )
    {
        debug() << __PRETTY_FUNCTION__ << device.udi() << "is not accessible"
                << "or has empty mount-point";
        return false;
    }
    return true;
}

SmbDeviceHandlerFactory::~SmbDeviceHandlerFactory( )
{
}

DeviceHandler *
SmbDeviceHandlerFactory::createHandler( KSharedConfigPtr, SqlStorage* ) const
{
    return 0;
}

DeviceHandler *
SmbDeviceHandlerFactory::createHandler( const Solid::Device &device, const QString &udi, SqlStorage *s ) const
{
    DEBUG_BLOCK
    if( !s )
    {
        debug() << "!s, returning 0";
        return 0;
    }
    if( !canHandle( device ) )
        return 0;

    const Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
    Q_ASSERT( access );  // canHandle() checks it
    QString mountPoint = access->filePath();

    const Solid::NetworkShare *netShare = device.as<Solid::NetworkShare>();
    Q_ASSERT( netShare );  // canHandle() checks it
    QUrl url = netShare->url(); // smb://testanot/share2
    QString server = url.host();
    QString share = url.path().mid( 1 ); // strip leading slash, not usual in smb shares

    QStringList ids = s->query( QString( "SELECT id, label, lastmountpoint "
                                         "FROM devices WHERE type = 'smb' "
                                         "AND servername = '%1' AND sharename = '%2';" )
                                         .arg( s->escape( server ) )
                                         .arg( s->escape( share ) ) );
    if ( ids.size() == 3 )
    {
        debug() << "Found existing SMB config for ID " << ids[0] << " , server " << server << " ,share " << share;
        s->query( QString( "UPDATE devices SET lastmountpoint = '%2' WHERE "
                           "id = %1;" )
                           .arg( ids[0] )
                           .arg( s->escape( mountPoint ) ) );
        return new SmbDeviceHandler( ids[0].toInt(), server, share, mountPoint, udi );
    }
    else
    {
        int id = s->insert( QString( "INSERT INTO devices"
                                     "( type, servername, sharename, lastmountpoint ) "
                                     "VALUES ( 'smb', '%1', '%2', '%3' );" )
                                     .arg( s->escape( server ) )
                                     .arg( s->escape( share ) )
                                     .arg( s->escape( mountPoint ) ),
                                     "devices" );
        if ( id == 0 )
        {
            warning() << "Inserting into devices failed for type=smb, server=" << server << ", share=" << share;
            return 0;
        }
        debug() << "Created new SMB device with ID " << id << " , server " << server << " ,share " << share;
        return new SmbDeviceHandler( id, server, share, mountPoint, udi );
    }
}
