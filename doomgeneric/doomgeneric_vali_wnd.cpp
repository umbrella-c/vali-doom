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

#include "doomgeneric_vali_wnd.hpp"

extern void addKeyToQueue(int pressed, unsigned char keyCode, char translated);

void DoomWindow::OnCreated()
{
    // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
    auto screenSize = GetScreen()->GetCurrentWidth() * GetScreen()->GetCurrentHeight() * 4;
    m_memory = Asgaard::MemoryPool::Create(this, screenSize);

    // Create initial buffer the size of this surface
    m_buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
        Dimensions().Height(), Asgaard::PixelFormat::A8B8G8R8);
    
    // Now all resources are created
    SetDropShadow(Asgaard::Rectangle(-10, -10, 20, 30));
    SetBuffer(m_buffer);
    OnRefreshed(m_buffer.get());
    ResetBuffer();
    RequestRedraw();
}

void DoomWindow::OnRefreshed(const Asgaard::MemoryBuffer* buffer)
{
    // Request redraw
    if (m_redraw) {
        Redraw();
        m_redraw = false;
    }
    else {
        m_redrawReady.store(true);
    }
}

void DoomWindow::OnKeyEvent(const Asgaard::KeyEvent& keyEvent)
{
    addKeyToQueue((int)keyEvent.Pressed(), keyEvent.KeyCode(), (char)(keyEvent.Key() & 0xFF));
}

void DoomWindow::ResetBuffer()
{
    Asgaard::Drawing::Painter paint(m_buffer);
    
    //paint.SetColor(0xFA, 0xEF, 0xDD);
    paint.SetFillColor(0xF0, 0xF0, 0xF0);
    paint.RenderFill();
}

void DoomWindow::UpdateBuffer(uint32_t* buffer)
{
    auto size = Dimensions().Width() * Dimensions().Height() * 4;
    if (m_buffer)
    {
        memcpy(m_buffer->Buffer(), buffer, size);
        RequestRedraw();
    }
}

void DoomWindow::UpdateTitle(const char* title)
{
    std::string cppTitle(title);
    SetTitle(cppTitle);
}

void DoomWindow::RequestRedraw()
{
    bool shouldRedraw = m_redrawReady.exchange(false);
    if (shouldRedraw) {
        Redraw();
    }
    else {
        m_redraw = true;
    }
}

void DoomWindow::Redraw()
{
    MarkDamaged(Dimensions());
    ApplyChanges();
}
