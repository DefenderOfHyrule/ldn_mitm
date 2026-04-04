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

#pragma once
#include <stratosphere.hpp>

namespace ams::mitm::ldn {

    // lazy nifm session management to avoid exhausting the limited nifm:a sessions (max 2)
    // this allows other system components (like Internet Settings) to acquire sessions
    
    class NifmSessionManager {
        private:
            static os::SdkMutex g_mutex;
            static bool g_initialized;
            static int g_refcount;
            
        public:
            // acquire a nifm session. uses reference counting to support nested calls.
            static Result Acquire();
            
            // release a nifm session. only actually closes when refcount reaches 0.
            static void Release();
            
            // force release, for emergency cleanup (e.g., in destructors)
            static void ForceRelease();
            
            // check if currently initialized
            static bool IsInitialized() { return g_initialized; }
            
            // get current refcount (only for debugging :P)
            static int GetRefCount() { return g_refcount; }
    };
    
    // for short-lived operations that need nifm
    class ScopedNifmSession {
        private:
            Result rc;
            bool acquired;
            
        public:
            ScopedNifmSession() : acquired(false) {
                rc = NifmSessionManager::Acquire();
                if (R_SUCCEEDED(rc)) {
                    acquired = true;
                }
            }
            
            ~ScopedNifmSession() {
                if (acquired) {
                    NifmSessionManager::Release();
                }
            }
            
            Result GetResult() const { return rc; }
            bool IsSucceeded() const { return R_SUCCEEDED(rc); }
    };

}
