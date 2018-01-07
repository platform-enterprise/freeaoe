/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SfmlRenderTarget.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "GraphicRender.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "mechanics/Map.h"

#define SCALE 3.

SfmlRenderTarget::SfmlRenderTarget(sf::RenderTarget &render_target)
{
    renderTarget_ = &render_target;
}

SfmlRenderTarget::~SfmlRenderTarget()
{
}

Vector2u SfmlRenderTarget::getSize(void) const
{
    return sf::Vector2u(renderTarget_->getSize().x / SCALE, renderTarget_->getSize().y / SCALE);
}

void SfmlRenderTarget::draw(EntityForm &form)
{
    form.getComponent<comp::GraphicRender>(comp::GRAPHIC_RENDER)->drawOn(*this);

    if (form.getRoot()->selected) {
        comp::MapObjectPtr mapObject = form.getRoot()->getComponent<comp::MapObject>(comp::MAP_OBJECT);

        ScreenPos pos = absoluteScreenPos(mapObject->getPos());
        pos.x -= Map::TILE_SIZE_HORIZONTAL / 8;
        pos.y -= Map::TILE_SIZE_VERTICAL;

        sf::RectangleShape rect;
        rect.setFillColor(sf::Color::Green);
        rect.setOutlineColor(sf::Color::Transparent);

        rect.setPosition(pos);
        rect.setSize(sf::Vector2f(Map::TILE_SIZE_HORIZONTAL / 4, 2));
        renderTarget_->draw(rect);
    }
}

void SfmlRenderTarget::draw(res::GraphicPtr graph, ScreenPos pos, int frame, float angle)
{
    for (const res::GraphicPtr delta : graph->getDeltas()) {
        draw(delta->getImage(frame, false, angle), pos - delta->getHotspot(frame) - delta->offset_);
    }

    draw(graph->getImage(frame, false, angle), pos - graph->getHotspot(frame));
}

void SfmlRenderTarget::draw(const sf::Image &image, ScreenPos pos)
{
    sf::Texture texture;


    texture.loadFromImage(image);

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);

    sprite.setPosition(pos);

    renderTarget_->draw(sprite);
}

void SfmlRenderTarget::draw(const sf::Texture &texture, ScreenPos pos)
{
    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(pos);

    renderTarget_->draw(sprite);
}

void SfmlRenderTarget::draw(const sf::Shape &shape)
{
    renderTarget_->draw(shape);
}

void SfmlRenderTarget::display(void)
{
    //  render_window_->Display();
}
