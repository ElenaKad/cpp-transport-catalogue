#include "svg.h"
#include <iostream>
#include <sstream>
namespace svg {

	using namespace std::literals;

	std::ostream& operator<<(std::ostream& os, const StrokeLineCap& cap) {
		switch (cap) {
		case StrokeLineCap::BUTT: os << "butt"; break;
		case StrokeLineCap::ROUND: os << "round"; break;
		case StrokeLineCap::SQUARE: os << "square"; break;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& join) {
		switch (join) {
		case StrokeLineJoin::ARCS: os << "arcs"; break;
		case StrokeLineJoin::BEVEL: os << "bevel"; break;
		case StrokeLineJoin::MITER: os << "miter"; break;
		case StrokeLineJoin::MITER_CLIP: os << "miter-clip"; break;
		case StrokeLineJoin::ROUND: os << "round"; break;
		}
		return os;
	}



	std::ostream& operator<<(std::ostream& os, const Color& color) {
		std::ostringstream out;
		std::visit(ColorPrinter{ out }, color);
		os << out.str();
		return os;
	}


	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}


	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\" "sv;
		RenderAttrs(context.out);
		out << "/>"sv;
	}

	//--------------------Polyline----------------

		// Добавляет очередную вершину к ломаной линии
	Polyline& Polyline::AddPoint(Point point) {
		points_.push_back(point);
		return *this;
	}


	// Отрисовывает ломаную линию
	void Polyline::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<polyline points=\"";
		for (auto it = points_.begin(); it != points_.end(); ++it) {
			out << it->x << "," << it->y;
			if (it != points_.end() - 1) {
				out << " ";
			}
		}


		out << "\" ";

		RenderAttrs(out);
		//out << ' ';
		out << "/>"sv;

	}


	//----------------------Text--------------------------
	
	// Задаёт координаты опорной точки (атрибуты x и y)
	Text& Text::SetPosition(Point pos) {
		x = pos.x;
		y = pos.y;
		return *this;
	}

	// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
	Text& Text::SetOffset(Point offset) {
		dx = offset.x;
		dy = offset.y;
		return *this;
	}

	// Задаёт размеры шрифта (атрибут font-size)
	Text& Text::SetFontSize(uint32_t size) {
		font_size = size;
		return *this;
	}

	// Задаёт название шрифта (атрибут font-family)
	Text& Text::SetFontFamily(std::string font_family) {
		font_family_name = font_family;
		return *this;
	}

	// Задаёт толщину шрифта (атрибут font-weight)
	Text& Text::SetFontWeight(std::string font_weight) {
		font_weight_type = font_weight;
		return *this;
	}

	// Задаёт текстовое содержимое объекта (отображается внутри тега text)
	Text& Text::SetData(std::string data) {
		text_data = data;
		return *this;
	}


	void Text::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<text ";
		RenderAttrs(context.out);
		out << " x=\"" << x << "\" y=\"" << y << "\"";

		out << " dx=\"" << dx << "\" dy=\"" << dy << "\"";

		out << " font-size=\"" << font_size << "\"";
		if (!font_family_name.empty()) {
			out << " font-family=\"" << font_family_name << "\"";
		}
		if (!font_weight_type.empty()) {
			out << " font-weight=\"" << font_weight_type << "\"";
		}

		out << ">" << text_data << "</text>";

	}


	//---------------------------Document------------------------


		// Добавляет в svg-документ объект-наследник svg::Object
	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects_.emplace_back(std::move(obj));
	}

	// Выводит в ostream svg-представление документа
	void Document::Render(std::ostream& out) const {
		out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
		out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;
		RenderContext context(out, 2);
		for (const auto& obj : objects_) {
			obj->Render(context.Indented());
		}
		out << "</svg>" << std::endl;

	}
}  // namespace svg