// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMessageBox>
#include <QString>

#include "citra_qt/multiplayer/message.h"
#include "citra_qt/util/sbs_dialog.h"

namespace NetworkMessage {
const ConnectionError USERNAME_NOT_VALID(
    QT_TR_NOOP("Username is not valid. Must be 4 to 20 alphanumeric characters."));
const ConnectionError ROOMNAME_NOT_VALID(
    QT_TR_NOOP("Room name is not valid. Must be 4 to 20 alphanumeric characters."));
const ConnectionError USERNAME_NOT_VALID_SERVER(
    QT_TR_NOOP("Username is already in use or not valid. Please choose another."));
const ConnectionError IP_ADDRESS_NOT_VALID(QT_TR_NOOP("IP is not a valid IPv4 address."));
const ConnectionError PORT_NOT_VALID(QT_TR_NOOP("Port must be a number between 0 to 65535."));
const ConnectionError NO_INTERNET(
    QT_TR_NOOP("Unable to find an internet connection. Check your internet settings."));
const ConnectionError UNABLE_TO_CONNECT(
    QT_TR_NOOP("Unable to connect to the host. Verify that the connection settings are correct. If "
               "you still cannot connect, contact the room host and verify that the host is "
               "properly configured with the external port forwarded."));
const ConnectionError ROOM_IS_FULL(
    QT_TR_NOOP("Unable to connect to the room because it is already full."));
const ConnectionError COULD_NOT_CREATE_ROOM(
    QT_TR_NOOP("Creating a room failed. Please retry. Restarting Citra might be necessary."));
const ConnectionError HOST_BANNED(
    QT_TR_NOOP("The host of the room has banned you. Speak with the host to unban you "
               "or try a different room."));
const ConnectionError WRONG_VERSION(
    QT_TR_NOOP("Version mismatch! Please update to the latest version of Citra. If the problem "
               "persists, contact the room host and ask them to update the server."));
const ConnectionError WRONG_PASSWORD(QT_TR_NOOP("Incorrect password."));
const ConnectionError GENERIC_ERROR(
    QT_TR_NOOP("An unknown error occured. If this error continues to occur, please open an issue"));
const ConnectionError LOST_CONNECTION(QT_TR_NOOP("Connection to room lost. Try to reconnect."));
const ConnectionError HOST_KICKED(QT_TR_NOOP("You have been kicked by the room host."));
const ConnectionError MAC_COLLISION(
    QT_TR_NOOP("MAC address is already in use. Please choose another."));
const ConnectionError CONSOLE_ID_COLLISION(QT_TR_NOOP(
    "Your Console ID conflicted with someone else's in the room.\n\nPlease go to Emulation "
    "> Configure > System to regenerate your Console ID."));
const ConnectionError PERMISSION_DENIED(
    QT_TR_NOOP("You do not have enough permission to perform this action."));
const ConnectionError NO_SUCH_USER(QT_TR_NOOP(
    "The user you are trying to kick/ban could not be found.\nThey may have left the room."));

static bool WarnMessage(const std::string& title, const std::string& text) {
    return MessageBox3D::Ok == MessageBox3D::warning(nullptr, QObject::tr(title.c_str()),
                                                     QObject::tr(text.c_str()),
                                                     MessageBox3D::Ok | MessageBox3D::Cancel);
}

void ShowError(const ConnectionError& e) {
    MessageBox3D::critical(nullptr, QObject::tr("Error"), QObject::tr(e.GetString().c_str()));
}

bool WarnCloseRoom() {
    return WarnMessage(
        QT_TR_NOOP("Leave Room"),
        QT_TR_NOOP("You are about to close the room. Any network connections will be closed."));
}

bool WarnDisconnect() {
    return WarnMessage(
        QT_TR_NOOP("Disconnect"),
        QT_TR_NOOP("You are about to leave the room. Any network connections will be closed."));
}

} // namespace NetworkMessage
