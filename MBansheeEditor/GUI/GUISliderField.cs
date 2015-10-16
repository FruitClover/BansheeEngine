﻿using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Editor GUI element that displays a slider with floating point input field and an optional label.
    /// </summary>
    public sealed class GUISliderField : GUIElement
    {
        public delegate void OnChangedDelegate(float newValue);

        /// <summary>
        /// Triggered when the value in the field changes.
        /// </summary>
        public event OnChangedDelegate OnChanged;

        /// <summary>
        /// Value displayed by the field input box.
        /// </summary>
        public float Value
        {
            get { return Internal_GetValue(mCachedPtr); }
            set { Internal_SetValue(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new slider field element with a label.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp values to.</param>
        /// <param name="max">Maximum boundary of the range to clamp values to.</param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUISliderField(float min, float max, GUIContent title, int titleWidth = 100, 
            string style = "", params GUIOption[] options)
        {
            Internal_CreateInstance(this, min, max, title, titleWidth, style, options, true);
        }

        /// <summary>
        /// Creates a new slider field element without a label.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp values to.</param>
        /// <param name="max">Maximum boundary of the range to clamp values to.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUISliderField(float min, float max, string style = "", params GUIOption[] options)
        {
            Internal_CreateInstance(this, min, max, null, 0, style, options, false);
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, color);
        }

        /// <summary>
        /// Sets a range that will input field values will be clamped to. Set to large negative/positive values if clamping
        /// is not required.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp values to.</param>
        /// <param name="max">Maximum boundary of the range to clamp values to.</param>
        public void SetRange(float min, float max)
        {
            Internal_SetRange(mCachedPtr, min, max);
        }

        /// <summary>
        /// Sets a step value that determines the minimal increment the slider can be increased or decreased by.
        /// </summary>
        /// <param name="step">Step value in percent if range is not defined, otherwise in same units as the range.</param>
        public void SetStep(float step)
        {
            Internal_SetStep(mCachedPtr, step);
        }

        /// <summary>
        /// Triggered by the runtime when the value of the float field changes.
        /// </summary>
        /// <param name="newValue">New value of the float field.</param>
        private void DoOnChanged(float newValue)
        {
            if (OnChanged != null)
                OnChanged(newValue);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUISliderField instance, float min, float max, 
            GUIContent title, int titleWidth, string style, GUIOption[] options, bool withTitle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetValue(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetValue(IntPtr nativeInstance, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRange(IntPtr nativeInstance, float min, float max);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStep(IntPtr nativeInstance, float step);
    }
}
