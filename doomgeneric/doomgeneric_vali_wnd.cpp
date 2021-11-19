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

DoomWindowContent::DoomWindowContent(uint32_t id, const std::shared_ptr<Asgaard::Screen>& screen, const Asgaard::Rectangle& dimensions)
    : SubSurface(id, screen, dimensions)
{
    // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
    auto screenSize = GetScreen()->GetCurrentWidth() * GetScreen()->GetCurrentHeight() * 4;
    m_memory = Asgaard::MemoryPool::Create(this, screenSize);

    // Create initial buffer the size of this surface
    m_buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
        Dimensions().Height(), Asgaard::PixelFormat::X8B8G8R8);
    SetBuffer(m_buffer);
}

void DoomWindowContent::OnKeyEvent(const Asgaard::KeyEvent& keyEvent)
{
    addKeyToQueue((int)keyEvent.Pressed(), keyEvent.KeyCode(), (char)(keyEvent.Key() & 0xFF));
}

void DoomWindowContent::UpdateBuffer(uint32_t* buffer)
{
    auto size = Dimensions().Width() * Dimensions().Height() * 4;
    memcpy(m_buffer->Buffer(), buffer, size);
}

void DoomWindowContent::Redraw()
{
    MarkDamaged(Dimensions());
    ApplyChanges();
}

DoomWindow::DoomWindow(uint32_t id, const std::shared_ptr<Asgaard::Screen>& screen, const Asgaard::Rectangle& dimensions)
    : WindowBase(id, screen, 
        Asgaard::Rectangle(dimensions.X(), dimensions.Y(), 
            dimensions.Width(), dimensions.Height() + TOPBAR_HEIGHT_VIOARR))
    , m_memory(nullptr)
    , m_buffer(nullptr)
    , m_content(nullptr)
    , m_redraw(false)
    , m_redrawReady(false)
{
    
}

void DoomWindow::OnCreated()
{
    auto fillBuffer = [](const auto& buffer)
    {
        Asgaard::Drawing::Painter paint(buffer);
        paint.SetFillColor(0, 0, 0);
        paint.RenderFill();
    };

    // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
    auto screenSize = GetScreen()->GetCurrentWidth() * GetScreen()->GetCurrentHeight() * 4;
    m_memory = Asgaard::MemoryPool::Create(this, screenSize);
    
    // Create initial buffer the size of this surface
    m_buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
        Dimensions().Height(), Asgaard::PixelFormat::X8B8G8R8, Asgaard::MemoryBuffer::Flags::NONE);

    // Create the content surface
    m_content = Asgaard::SubSurface::Create<DoomWindowContent>(this, 
        Asgaard::Rectangle(0, TOPBAR_HEIGHT_VIOARR, Dimensions().Width(), Dimensions().Height() - TOPBAR_HEIGHT_VIOARR));

    SetTitle("doom");
    EnableDecoration(true);
    fillBuffer(m_buffer);

    SetDropShadow(Asgaard::Rectangle(-10, -10, 20, 30));
    SetBuffer(m_buffer);
    OnRefreshed(m_buffer.get());
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

void DoomWindow::UpdateBuffer(uint32_t* buffer)
{
    if (m_content) {
        m_content->UpdateBuffer(buffer);
    }
    RequestRedraw();
}

void DoomWindow::UpdateTitle(const char* title)
{
    SetTitle(std::string(title));
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
    if (m_content) {
        m_content->Redraw();
    }
    MarkDamaged(Dimensions());
    ApplyChanges();
}
