/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla IPC.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@netscape.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <string.h>
#include "ipcLog.h"
#include "ipcCommandModule.h"
#include "ipcModule.h"
#include "ipcClient.h"
#include "ipcMessage.h"
#include "ipcMessageUtils.h"
#include "ipcd.h"
#include "ipcm.h"

struct ipcCommandModule
{
    typedef void (* MsgHandler)(ipcClient *, const ipcMessage *);

    //
    // message handlers
    //

    static void OnPing(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got PING\n"));

        IPC_SendMsg(client, new ipcmMessagePing());
    }

    static void OnClientHello(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got CLIENT_HELLO\n"));

        ipcMessageCast<ipcmMessageClientHello> msg(rawMsg);
        const char *name = msg->PrimaryName();
        if (name)
            client->AddName(name);

        IPC_SendMsg(client, new ipcmMessageClientID(client->ID()));
    }

    static void OnClientAddName(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got CLIENT_ADD_NAME\n"));

        ipcMessageCast<ipcmMessageClientAddName> msg(rawMsg);
        const char *name = msg->Name();
        if (name)
            client->AddName(name);
    }

    static void OnClientDelName(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got CLIENT_DEL_NAME\n"));

        ipcMessageCast<ipcmMessageClientDelName> msg(rawMsg);
        const char *name = msg->Name();
        if (name)
            client->DelName(name);
    }

    static void OnClientAddTarget(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got CLIENT_ADD_TARGET\n"));

        ipcMessageCast<ipcmMessageClientAddTarget> msg(rawMsg);
        client->AddTarget(msg->Target());
    }

    static void OnClientDelTarget(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got CLIENT_DEL_TARGET\n"));

        ipcMessageCast<ipcmMessageClientDelTarget> msg(rawMsg);
        client->DelTarget(msg->Target());
    }

    static void OnQueryClientByName(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got QUERY_CLIENT_BY_NAME\n"));

        ipcMessageCast<ipcmMessageQueryClientByName> msg(rawMsg);
        ipcClient *result = IPC_GetClientByName(msg->Name());
        if (result) {
            LOG(("  client exists w/ ID = %u\n", result->ID()));
            IPC_SendMsg(client, new ipcmMessageClientID(result->ID()));
        }
        else {
            LOG(("  client does not exist\n"));
            IPC_SendMsg(client, new ipcmMessageError(IPCM_ERROR_CLIENT_NOT_FOUND));
        }
    }

    static void OnForward(ipcClient *client, const ipcMessage *rawMsg)
    {
        LOG(("got FORWARD\n"));

        ipcMessageCast<ipcmMessageForward> msg(rawMsg);
        PRUint32 destID = msg->DestClientID();

        ipcMessage *newMsg = new ipcMessage();
        newMsg->Init(msg->InnerTarget(),
                     msg->InnerData(),
                     msg->InnerDataLen());

        ipcClient *dest = IPC_GetClientByID(destID);
        if (!dest) {
            LOG(("  destination client not found!\n"));
            return;
        }
        IPC_SendMsg(dest, newMsg);
    }

    //
    // ipcModule interface impl
    //

    static void Init()
    {
    }

    static void Shutdown()
    {
    }

    static void HandleMsg(ipcClient *client, const ipcMessage *rawMsg)
    {
        static MsgHandler handlers[] =
        {
            OnPing,
            NULL, // ERROR
            OnClientHello,
            NULL, // CLIENT_ID
            NULL, // CLIENT_INFO
            OnClientAddName,
            OnClientDelName,
            OnClientAddTarget,
            OnClientDelTarget,
            OnQueryClientByName,
            NULL, // QUERY_CLIENT_INFO
            OnForward,
        };

        int type = IPCM_GetMsgType(rawMsg);
        LOG(("ipcCommandModule::HandleMsg [type=%d]\n", type));

        if (type < IPCM_MSG_TYPE_UNKNOWN) {
            if (handlers[type]) {
                MsgHandler handler = handlers[type];
                handler(client, rawMsg);
            }
        }
    }
};

ipcModuleMethods *IPC_GetCommandModuleMethods()
{
    static ipcModuleMethods methods =
    {
        IPC_MODULE_METHODS_VERSION,
        ipcCommandModule::Init,
        ipcCommandModule::Shutdown,
        ipcCommandModule::HandleMsg
    };
    return &methods;
}
