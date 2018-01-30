#include "gui/elements/GUIImageDisplay.h"
#include "gui/GUIRenderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::gui::ImageDisplay, "lux.gui.ImageDisplay")

namespace lux
{
namespace gui
{

ImageDisplay::ImageDisplay() :
	m_StretchToFit(false)
{
	SetFocusable(false);
}

ImageDisplay::~ImageDisplay()
{
}

void ImageDisplay::SetTexture(video::Texture* texture)
{
	m_Texture = texture;
}

video::Texture* ImageDisplay::GetTexture() const
{
	return m_Texture;
}

void ImageDisplay::SetOverlayTexture(video::Texture* texture)
{
	m_OverlayTexture = texture;
}

video::Texture* ImageDisplay::GetOverlayTexture() const
{
	return m_OverlayTexture;
}

void ImageDisplay::SetStretchToFit(bool stretch)
{
	m_StretchToFit = stretch;
}

bool ImageDisplay::GetStretchToFit() const
{
	return m_StretchToFit;
}

void ImageDisplay::Paint(Renderer* renderer, float secsPassed)
{
	LUX_UNUSED(secsPassed);

	DrawTexture(m_Texture, renderer);
	if(m_OverlayTexture)
		DrawTexture(m_OverlayTexture, renderer);
}

void ImageDisplay::DrawTexture(video::Texture* tex, Renderer* renderer)
{
	auto final = GetFinalRect();
	auto backColor = GetFinalPalette().GetWindow();
	if(!tex) {
		renderer->DrawRectangle(final, backColor);
		return;
	}

	auto imgSize = tex->GetSize();
	auto rectSize = math::Dimension2F(final.GetWidth(), final.GetHeight());

	if(!m_StretchToFit) {
		if(imgSize.width * rectSize.height < imgSize.height * rectSize.width) {
			float err = rectSize.width - (imgSize.width * (rectSize.height / imgSize.height));
			if(err) {
				renderer->DrawRectangle(math::RectF(final.left, final.top, final.left + err / 2, final.bottom), backColor);
				renderer->DrawRectangle(math::RectF(final.right - err / 2, final.top, final.right, final.bottom), backColor);
			}
			renderer->DrawRectangle(math::RectF(final.left + err / 2, final.top, final.right - err / 2, final.bottom), tex,
				math::RectF(0, 0, 1, 1), video::Color::White);
		} else {
			float err = rectSize.height - (imgSize.height * (rectSize.width / imgSize.width));
			if(err) {
				renderer->DrawRectangle(math::RectF(final.left, final.top, final.right, final.top + err / 2), backColor);
				renderer->DrawRectangle(math::RectF(final.left, final.bottom - err / 2, final.right, final.bottom), backColor);
			}
			renderer->DrawRectangle(math::RectF(final.left, final.top + err / 2, final.right, final.bottom - err / 2), tex,
				math::RectF(0, 0, 1, 1), video::Color::White);
		}
	} else {
		renderer->DrawRectangle(math::RectF(final.left, final.top, final.right, final.bottom), tex,
			math::RectF(0, 0, 1, 1), video::Color::White);
	}
}

} // namespace gui
} // namespace lux