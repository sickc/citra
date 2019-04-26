// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QStandardItemModel>
#include "citra_qt/game_list.h"
#include "citra_qt/multiplayer/client_room.h"
#include "citra_qt/multiplayer/direct_connect.h"
#include "citra_qt/multiplayer/host_room.h"
#include "citra_qt/multiplayer/lobby.h"
#include "citra_qt/multiplayer/message.h"
#include "citra_qt/multiplayer/state.h"
#include "citra_qt/sbs_3d_ui.h"
#include "citra_qt/ui_settings.h"
#include "citra_qt/util/clickable_label.h"
#include "common/announce_multiplayer_room.h"
#include "common/logging/log.h"

MultiplayerState::MultiplayerState(QWidget* parent, QStandardItemModel* game_list_model,
                                   QAction* leave_room, QAction* show_room)
    : QWidget(parent), game_list_model(game_list_model), leave_room(leave_room),
      show_room(show_room) {
    if (auto member = Network::GetRoomMember().lock()) {
        // register the network structs to use in slots and signals
        state_callback_handle = member->BindOnStateChanged(
            [this](const Network::RoomMember::State& state) { emit NetworkStateChanged(state); });
        connect(this, &MultiplayerState::NetworkStateChanged, this,
                &MultiplayerState::OnNetworkStateChanged);
        error_callback_handle = member->BindOnError(
            [this](const Network::RoomMember::Error& error) { emit NetworkError(error); });
        connect(this, &MultiplayerState::NetworkError, this, &MultiplayerState::OnNetworkError);
    }

    qRegisterMetaType<Network::RoomMember::State>();
    qRegisterMetaType<Network::RoomMember::Error>();
    qRegisterMetaType<Common::WebResult>();
    announce_multiplayer_session = std::make_shared<Core::AnnounceMultiplayerSession>();
    announce_multiplayer_session->BindErrorCallback(
        [this](const Common::WebResult& result) { emit AnnounceFailed(result); });
    connect(this, &MultiplayerState::AnnounceFailed, this, &MultiplayerState::OnAnnounceFailed);

    status_text = new ClickableLabel(this);
    status_icon = new ClickableLabel(this);
    status_text->setToolTip(tr("Current connection status"));
    status_text->setText(tr("Not Connected. Click here to find a room!"));
    status_icon->setPixmap(QIcon::fromTheme("disconnected").pixmap(16));

    connect(status_text, &ClickableLabel::clicked, this, &MultiplayerState::OnOpenNetworkRoom);
    connect(status_icon, &ClickableLabel::clicked, this, &MultiplayerState::OnOpenNetworkRoom);

    connect(static_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged, this,
            [this](QWidget* /*old*/, QWidget* now) {
                if (client_room && client_room->isAncestorOf(now)) {
                    HideNotification();
                }
            });
}

MultiplayerState::~MultiplayerState() {
    if (state_callback_handle) {
        if (auto member = Network::GetRoomMember().lock()) {
            member->Unbind(state_callback_handle);
        }
    }

    if (error_callback_handle) {
        if (auto member = Network::GetRoomMember().lock()) {
            member->Unbind(error_callback_handle);
        }
    }
}

void MultiplayerState::Close() {
    if (host_room)
        host_room->close();
    if (direct_connect)
        direct_connect->close();
    if (client_room)
        client_room->close();
    if (lobby)
        lobby->close();
}

void MultiplayerState::retranslateUi() {
    status_text->setToolTip(tr("Current connection status"));

    if (current_state == Network::RoomMember::State::Uninitialized) {
        status_text->setText(tr("Not Connected. Click here to find a room!"));
    } else if (current_state == Network::RoomMember::State::Joined ||
               current_state == Network::RoomMember::State::Moderator) {

        status_text->setText(tr("Connected"));
    } else {
        status_text->setText(tr("Not Connected"));
    }

    if (lobby)
        lobby->RetranslateUi();
    if (host_room)
        host_room->RetranslateUi();
    if (client_room)
        client_room->RetranslateUi();
    if (direct_connect)
        direct_connect->RetranslateUi();
}

void MultiplayerState::OnNetworkStateChanged(const Network::RoomMember::State& state) {
    LOG_DEBUG(Frontend, "Network State: {}", Network::GetStateStr(state));
    if (state == Network::RoomMember::State::Joined ||
        state == Network::RoomMember::State::Moderator) {

        OnOpenNetworkRoom();
        status_icon->setPixmap(QIcon::fromTheme("connected").pixmap(16));
        status_text->setText(tr("Connected"));
        leave_room->setEnabled(true);
        show_room->setEnabled(true);
    } else {
        status_icon->setPixmap(QIcon::fromTheme("disconnected").pixmap(16));
        status_text->setText(tr("Not Connected"));
        leave_room->setEnabled(false);
        show_room->setEnabled(false);
    }

    current_state = state;
}

void MultiplayerState::OnNetworkError(const Network::RoomMember::Error& error) {
    LOG_DEBUG(Frontend, "Network Error: {}", Network::GetErrorStr(error));
    switch (error) {
    case Network::RoomMember::Error::LostConnection:
        NetworkMessage::ShowError(NetworkMessage::LOST_CONNECTION);
        break;
    case Network::RoomMember::Error::HostKicked:
        NetworkMessage::ShowError(NetworkMessage::HOST_KICKED);
        break;
    case Network::RoomMember::Error::CouldNotConnect:
        NetworkMessage::ShowError(NetworkMessage::UNABLE_TO_CONNECT);
        break;
    case Network::RoomMember::Error::NameCollision:
        NetworkMessage::ShowError(NetworkMessage::USERNAME_NOT_VALID_SERVER);
        break;
    case Network::RoomMember::Error::MacCollision:
        NetworkMessage::ShowError(NetworkMessage::MAC_COLLISION);
        break;
    case Network::RoomMember::Error::ConsoleIdCollision:
        NetworkMessage::ShowError(NetworkMessage::CONSOLE_ID_COLLISION);
        break;
    case Network::RoomMember::Error::RoomIsFull:
        NetworkMessage::ShowError(NetworkMessage::ROOM_IS_FULL);
        break;
    case Network::RoomMember::Error::WrongPassword:
        NetworkMessage::ShowError(NetworkMessage::WRONG_PASSWORD);
        break;
    case Network::RoomMember::Error::WrongVersion:
        NetworkMessage::ShowError(NetworkMessage::WRONG_VERSION);
        break;
    case Network::RoomMember::Error::HostBanned:
        NetworkMessage::ShowError(NetworkMessage::HOST_BANNED);
        break;
    case Network::RoomMember::Error::UnknownError:
        NetworkMessage::ShowError(NetworkMessage::UNABLE_TO_CONNECT);
        break;
    case Network::RoomMember::Error::PermissionDenied:
        NetworkMessage::ShowError(NetworkMessage::PERMISSION_DENIED);
        break;
    case Network::RoomMember::Error::NoSuchUser:
        NetworkMessage::ShowError(NetworkMessage::NO_SUCH_USER);
        break;
    }
}

void MultiplayerState::OnAnnounceFailed(const Common::WebResult& result) {
    announce_multiplayer_session->Stop();
    QMessageBox::warning(
        this, tr("Error"),
        tr("Failed to announce the room to the public lobby. In order to host a room publicly, you "
           "must have a valid Citra account configured in Emulation -> Configure -> Web. If you do "
           "not want to publish a room in the public lobby, then select Unlisted instead.\n"
           "Debug Message: ") +
            QString::fromStdString(result.result_string),
        QMessageBox::Ok);
}

void MultiplayerState::UpdateThemedIcons() {
    if (show_notification) {
        status_icon->setPixmap(QIcon::fromTheme("connected_notification").pixmap(16));
    } else if (current_state == Network::RoomMember::State::Joined ||
               current_state == Network::RoomMember::State::Moderator) {

        status_icon->setPixmap(QIcon::fromTheme("connected").pixmap(16));
    } else {
        status_icon->setPixmap(QIcon::fromTheme("disconnected").pixmap(16));
    }
    if (client_room)
        client_room->UpdateIconDisplay();
}

void MultiplayerState::BringWidgetToFront(QWidget* widget) {
    MoveDialogToLeftEye(widget, qobject_cast<QWidget*>(parent()));
    widget->show();
    widget->activateWindow();
    widget->raise();
}

void MultiplayerState::OnViewLobby() {
    if (lobby == nullptr) {
        lobby = new Lobby(this, game_list_model, announce_multiplayer_session);
    }
    BringWidgetToFront(lobby);
}

void MultiplayerState::OnCreateRoom() {
    if (host_room == nullptr) {
        host_room = new HostRoomWindow(this, game_list_model, announce_multiplayer_session);
    }
    BringWidgetToFront(host_room);
}

bool MultiplayerState::OnCloseRoom() {
    if (!NetworkMessage::WarnCloseRoom())
        return false;
    if (auto room = Network::GetRoom().lock()) {
        // if you are in a room, leave it
        if (auto member = Network::GetRoomMember().lock()) {
            member->Leave();
            LOG_DEBUG(Frontend, "Left the room (as a client)");
        }

        // if you are hosting a room, also stop hosting
        if (room->GetState() != Network::Room::State::Open) {
            return true;
        }
        // Save ban list
        if (auto room = Network::GetRoom().lock()) {
            UISettings::values.ban_list = std::move(room->GetBanList());
        }
        room->Destroy();
        announce_multiplayer_session->Stop();
        LOG_DEBUG(Frontend, "Closed the room (as a server)");
    }
    return true;
}

void MultiplayerState::ShowNotification() {
    if (client_room && client_room->isAncestorOf(QApplication::focusWidget()))
        return; // Do not show notification if the chat window currently has focus
    show_notification = true;
    QApplication::alert(nullptr);
    status_icon->setPixmap(QIcon::fromTheme("connected_notification").pixmap(16));
    status_text->setText(tr("New Messages Received"));
}

void MultiplayerState::HideNotification() {
    show_notification = false;
    status_icon->setPixmap(QIcon::fromTheme("connected").pixmap(16));
    status_text->setText(tr("Connected"));
}

void MultiplayerState::OnOpenNetworkRoom() {
    if (auto member = Network::GetRoomMember().lock()) {
        if (member->IsConnected()) {
            if (client_room == nullptr) {
                client_room = new ClientRoomWindow(this);
                connect(client_room, &ClientRoomWindow::ShowNotification, this,
                        &MultiplayerState::ShowNotification);
            }
            BringWidgetToFront(client_room);
            return;
        }
    }
    // If the user is not a member of a room, show the lobby instead.
    // This is currently only used on the clickable label in the status bar
    OnViewLobby();
}

void MultiplayerState::OnDirectConnectToRoom() {
    if (direct_connect == nullptr) {
        direct_connect = new DirectConnectWindow(this);
    }
    BringWidgetToFront(direct_connect);
}
