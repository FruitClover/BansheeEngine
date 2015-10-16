#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIFieldBase.h"

namespace BansheeEngine
{
	/**
	 * @brief	A composite GUI object representing an editor field. Editor fields are a combination
	 *			of a label and an input field. Label is optional. This specific implementation
	 *			displays a horizontal slider and a floating point input box.
	 */
	class BS_ED_EXPORT GUISliderField : public TGUIField<GUISliderField>
	{
	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Style type name for the internal input box.
		 */
		static const String& getInputStyleType();

		/**
		 * Style type name for the internal slider.
		 */
		static const String& getSliderStyleType();

		GUISliderField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**
		 * @brief	Returns the value of the input field/slider.
		 */
		float getValue() const;

		/**
		 * @brief	Sets a new value in the input field/slider.
		 */
		void setValue(float value);

		/**
		 * @brief	Sets a minimum and maximum allow values in the input field.
		 *			Set to large negative/positive values if you don't require clamping.
		 */
		void setRange(float min, float max);

		/**
		 * @brief	Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero
		 * 			to have no step.
		 */
		void setStep(float step);

		/**
		 * @copydoc	GUIElement::setTint
		 */
		virtual void setTint(const Color& color) override;

		Event<void(float)> onValueChanged; /**< Triggers when the field value changes. */
	protected:
		virtual ~GUISliderField();

		/**
		 * @copydoc	GUIElementContainer::styleUpdated
		 */
		void styleUpdated() override;

		/**
		 * @brief	Triggered when the input box value changes.
		 */
		void valueChanged(const WString& newValue);

		/**
		 * @brief	Triggered when the slider is moved.
		 */
		void sliderChanged(float newValue);

		/**
		 * @brief	Callback that checks can the provided string be
		 *			converted to a floating point value.
		 */
		static bool floatFilter(const WString& str);

		GUIInputBox* mInputBox;
		GUISliderHorz* mSlider;
	};
}