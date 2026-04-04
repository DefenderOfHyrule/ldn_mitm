/*
 * Copyright (c) 2024 ldn_mitm contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nifm_manager.hpp"
#include "debug.hpp"

namespace ams::mitm::ldn {

    os::SdkMutex NifmSessionManager::g_mutex;
    bool NifmSessionManager::g_initialized = false;
    int NifmSessionManager::g_refcount = 0;

    Result NifmSessionManager::Acquire() {
        std::scoped_lock lk(g_mutex);
        
        if (g_refcount == 0) {
            // first acquisition, initialize nifm with Admin privileges
            Result rc = nifmInitialize(NifmServiceType_Admin);
            if (R_FAILED(rc)) {
                LogFormat("NifmSessionManager: nifmInitialize failed: %x", rc);
                return rc;
            }
            g_initialized = true;
            LogFormat("NifmSessionManager: session acquired (refcount: 0 -> 1)");
        } else {
            LogFormat("NifmSessionManager: refcount incremented (%d -> %d)", g_refcount, g_refcount + 1);
        }
        
        g_refcount++;
        return ResultSuccess();
    }

    void NifmSessionManager::Release() {
        std::scoped_lock lk(g_mutex);
        
        if (g_refcount <= 0) {
            LogFormat("NifmSessionManager: WARNING - Release() called with refcount %d", g_refcount);
            return;
        }
        
        g_refcount--;
        
        if (g_refcount == 0 && g_initialized) {
            // last release, cleanup nifm session
            nifmExit();
            g_initialized = false;
            LogFormat("NifmSessionManager: session released (refcount: 1 -> 0)");
        } else if (g_refcount > 0) {
            LogFormat("NifmSessionManager: refcount decremented (%d -> %d)", g_refcount + 1, g_refcount);
        }
    }

    void NifmSessionManager::ForceRelease() {
        std::scoped_lock lk(g_mutex);
        
        if (g_initialized) {
            LogFormat("NifmSessionManager: FORCE RELEASE (refcount was: %d)", g_refcount);
            nifmExit();
            g_initialized = false;
            g_refcount = 0;
        }
    }

}
