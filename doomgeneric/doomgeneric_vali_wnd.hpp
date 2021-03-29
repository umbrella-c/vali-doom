/* ValiOS
 *
 * Copyright 2020, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Doom - Application Window
 */
#pragma once

#include <asgaard/application.hpp>
#include <asgaard/window_base.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/key_event.hpp>
#include <asgaard/drawing/painter.hpp>
#include <os/keycodes.h>

class DoomWindow final : public Asgaard::WindowBase {
public:
    DoomWindow(uint32_t id, const std::shared_ptr<Asgaard::Screen>& screen, const Asgaard::Rectangle& dimensions)
        : WindowBase(id, screen, dimensions)
        , m_memory(nullptr)
        , m_buffer(nullptr)
        , m_redraw(false)
        , m_redrawReady(false) { }
    
    ~DoomWindow() { }
    
    void UpdateBuffer(uint32_t* buffer);
    void UpdateTitle(const char* title);

private:
    void OnCreated() override;
    void OnRefreshed(Asgaard::MemoryBuffer* buffer) override;
    void OnKeyEvent(const Asgaard::KeyEvent& keyEvent) override;
    void Teardown() override;

private:
    void ResetBuffer();
    void RequestRedraw();
    void Redraw();
    
private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;

    bool m_redraw;
    std::atomic<bool> m_redrawReady;
};
