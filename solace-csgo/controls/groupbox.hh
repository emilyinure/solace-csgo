#pragma once
#include "base_control.hh"
#include "../input_helper/input_helper.hh"
#include <iostream>
#include <fstream>
class c_group_tab : public c_base_control
{
public:
    vector_2d drag_offset_{0, 0};
    c_base_control* selected_tab = nullptr;
    std::vector<std::shared_ptr<c_base_control>> children_{};

public:
    float offset = 0.f;
    float scroll_y = 0;
    explicit c_group_tab(const char* name_)
    {
        name = name_;
        type = control_type_tab;
    }
    auto draw() -> void override
    {
        // child handling.
        if (!this->children_.empty())
        {
            // handle position.
            vector_2d child_offset{0, 6};
            for (const auto& child : this->children_)
            {

                switch (child->type)
                {
                    case control_type_invalid: /* invalid control, do nothing. */
                        break;
                    case control_type_key_bind:
                    case control_type_slider:
                    case control_type_combobox:
                    {
                        child->adjust_area({this->area.x + 6, this->area.y + 2.f, this->area.w - 12, 32});
                        child->adjust_position(child_offset);
                        child_offset.y += 38;
                        break;
                    }
                    default:
                    {
                        child->adjust_area({this->area.x + 6, this->area.y + 2.f, this->area.w - 12, 16});
                        child->adjust_position(child_offset);
                        child_offset.y += 22;
                    }
                    break;
                }
            }
            offset = child_offset.y + 6;
            // draw.
            auto has_focus = false;
            for (const auto& child : this->children_)
            {
                if (child.get() == menu.focused_control)
                {
                    has_focus = true;
                    continue;
                }

                child->draw();
            }

            if (has_focus && menu.focused_control)
                menu.focused_control->draw();
        }
    }
    auto disable() -> void override
    {
        for (const auto& child : this->children_)
        {
            if (child.get() == menu.focused_control)
            {
                child->disable();
                menu.focused_control = nullptr;
                break;
            }
        }
    }
    auto finish()
    {
    }
    auto add_child(std::shared_ptr<c_base_control> control) -> void
    {
        this->children_.push_back(control);
    }

    auto update() -> void override
    {
        auto has_focus = false;
        for (const auto& child : this->children_)
        {
            if (child.get() == menu.focused_control)
            {
                has_focus = true;
                continue;
            }
        }

        if (has_focus && menu.focused_control)
            menu.focused_control->update();
        else
            for (const auto& child : this->children_)
            {
                child->update();
            }
    }
    void save() override
    {
        for (auto i : children_)
        {
            i->save();
        }
    }
    void load() override
    {
        for (auto i : children_)
        {
            i->load();
        }
    }
};
class c_group_box : public c_base_control
{
    vector_2d drag_offset_{0, 0};
    c_group_tab* selected_tab = nullptr;
    std::vector<std::shared_ptr<c_group_tab>> children_{};

public:
    float scroll_y = 0;

    explicit c_group_box(const char* name_)
    {
        name = name_;
        type = control_type_tab;
    }

    auto draw() -> void override
    {
        g.m_render->filled_rect(this->area.x, this->area.y, this->area.w, this->area.h, color{240, 240, 240, 7});
        auto text_height = g.m_render->get_text_height(this->name, g.m_render->m_constantia_12());
        g.m_render->text(g.m_render->m_constantia_12(), this->area.x + 5, this->area.y + 10 - (text_height / 2.f),
                         menu.main_theme, this->name);
        g.m_render->outlined_rect(this->area.x, this->area.y, this->area.w, this->area.h, {240, 240, 240, 7});
        g.m_render->filled_rect(this->area.x, this->area.y + 20, this->area.w, this->area.h - 20,
                                input_helper.hovering(this->area) ? color{240, 240, 240, 8} : color{240, 240, 240, 7});

        // child handling.
        if (!this->children_.empty())
        {
            // handle position.
            const vector_2d child_offset{0, scroll_y};
            for (const auto& child : this->children_)
            {
                child->adjust_area({this->area.x, this->area.y + 20, this->area.w, this->area.h - 20});
                child->adjust_position(child_offset);
            }

            if (children_.size() > 1)
            {
                auto text_x = this->area.x + this->area.w - 10.f;
                for (int i = children_.size() - 1; i >= 0; i--)
                {
                    auto* const child = children_[i].get();
                    const auto name_width = g.m_render->get_text_width(child->name, g.m_render->m_constantia_12());
                    text_height = g.m_render->get_text_height(this->name, g.m_render->m_constantia_12());
                    text_x -= name_width;
                    render_t::text(g.m_render->m_constantia_12(), text_x, this->area.y + 10 - (text_height / 2.f),
                                   child == selected_tab ? menu.main_theme : menu.bright, child->name);
                    text_x -= 7.f;
                }
            }

            // draw.
            auto has_focus = false;
            g.m_render->push_clip(this->area.x + 1, this->area.y + 20, this->area.w, this->area.h - 21);
            for (const auto& child : this->children_)
            {
                if (!selected_tab)
                    selected_tab = child.get();
                if (child.get() != selected_tab)
                {
                    continue;
                }

                child->draw();
            }

            g.m_render->pop_clip();
        }
        // this->area.h = ( 16 * this->children_.size( ) );
    }

    auto update() -> void override
    {
        // ugh... ghetto...
        for (const auto& child : this->children_)
        {
            if (child.get() == menu.focused_control)
            {
                child->disable();
                menu.focused_control = nullptr;
                break;
            }
        }
        if (input_helper.hovering({static_cast<float>(area.x), area.y, area.w, area.h}))
            scroll_y = fminf(0, roundf(scroll_y + input_helper.scroll()));
        if (this->children_.size() > 1)
        {
            int text_x = this->area.x + this->area.w - 6.f;
            for (int i = children_.size() - 1; i >= 0; i--)
            {
                const auto child = children_[i].get();
                const auto name_width = g.m_render->get_text_width(child->name, g.m_render->m_constantia_12());
                const auto name_height = g.m_render->get_text_height(child->name, g.m_render->m_constantia_12());
                text_x -= int(name_width + 7.f);
                if (input_helper.hovering({static_cast<float>(text_x), this->area.y + 1, static_cast<float>(name_width),
                                           static_cast<float>(name_height)}) &&
                    input_helper.key_pressed(VK_LBUTTON))
                {
                    input_helper.set_key(VK_LBUTTON, false);
                    selected_tab = child;
                    for (auto i : children_)
                        if (i.get() != selected_tab)
                            i->disable();
                    break;
                }
            }
        }
        if (this->selected_tab != nullptr)
        {
            // if ( input_helper.hovering( { this->area.x + 1, this->area.y + 20, this->area.w - 2, this->area.h - 21.f
            // } ) )
            float max_scroll = fmaxf(0,this->selected_tab->offset - (this->area.h - 20));
            if (-max_scroll > scroll_y)
                scroll_y = -max_scroll;
            this->selected_tab->update();
        }
    }
    auto disable() -> void override
    {
        for (const auto& child : this->children_)
        {
            child->disable();
        }
    }
    auto finish()
    {
    }
    auto add_child(std::shared_ptr<c_group_tab> control) -> void
    {
        this->children_.push_back(control);
    }
    void save() override
    {
        for (auto i : children_)
        {
            i->save();
        }
    }
    void load() override
    {
        for (auto i : children_)
        {
            i->load();
        }
    }
};