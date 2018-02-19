#ifndef INCLUDED_GUI_IMAGE_DISPLAY_H
#define INCLUDED_GUI_IMAGE_DISPLAY_H
#include "gui/GUIElement.h"
#include "video/Texture.h"

namespace lux
{
namespace gui
{

class ImageDisplay : public Element
{
	LX_REFERABLE_MEMBERS_API(ImageDisplay, LUX_API);
public:
	LUX_API ImageDisplay();
	LUX_API ~ImageDisplay();

	LUX_API void SetTexture(video::Texture* texture);
	LUX_API video::Texture* GetTexture() const;
	LUX_API void SetOverlayTexture(video::Texture* texture);
	LUX_API video::Texture* GetOverlayTexture() const;

	LUX_API void SetStretchToFit(bool stretch);
	LUX_API bool GetStretchToFit() const;

	LUX_API void Paint(Renderer* renderer, float secsPassed);

private:
	void DrawTexture(video::Texture* tex, Renderer* renderer);

private:
	StrongRef<video::Texture> m_Texture;
	StrongRef<video::Texture> m_OverlayTexture;

	bool m_StretchToFit;
};

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_IMAGE_DISPLAY_H