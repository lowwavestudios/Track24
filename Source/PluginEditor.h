/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LWSLookAndFeel.h"

//==============================================================================
/**
*/

class Track24AudioProcessorEditor  : public juce::AudioProcessorEditor, public Timer, public Slider::Listener, public Button::Listener, public ComboBox::Listener
{
public:
    Track24AudioProcessorEditor (Track24AudioProcessor&);
    ~Track24AudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;

    void sliderValueChanged(Slider* sliderThatHasChanged) override;
    void buttonClicked(Button* buttonThatHasChanged) override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void paintOverChildren(Graphics& g) override;

    void newButtonStyle(Colour textColour, Colour tickColour, Colour disableColour);

    static const char* logoSmall_png;
    static const int logoSmall_pngSize;

    Image cachedImage_logo2020_png2_1;

    void paramTextStyle(Graphics& g, String name, float x, float y, float width, float fontHeight, bool drawBox, Justification justification, Colour textColour = Colour(245, 245, 245));
    LWSLookAndFeel::MeterLookAndFeel meterLAF;
    LWSLookAndFeel::MeterLookAndFeel meterLAF2;
    LWSLookAndFeel::buttonLookAndFeel buttonLAF;
    LWSLookAndFeel::LWSComboBox comboBoxLAF;
    LWSLookAndFeel::ReverseSliderLookAndFeel reverseSliderLAF;
    LWSLookAndFeel::RotaryLookAndFeel rotaryLAF;

    Font LWSFont;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Track24AudioProcessor& audioProcessor;

    
    Slider meterL, meterR, gainSlider, hpfSlider, lpfSlider, textureSlider, dryWetSlider, outputSlider;
    ToggleButton oversampleButton;
    ComboBox toneBox;
    

    std::unique_ptr<HyperlinkButton> help;

    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        gainSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        hpfSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        lpfSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        textureSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        dryWetSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>
        oversampleButtonValue;
    std::unique_ptr< AudioProcessorValueTreeState::ComboBoxAttachment>
        toneBoxValue;
    std::unique_ptr< AudioProcessorValueTreeState::SliderAttachment>
        outputSliderValue;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Track24AudioProcessorEditor)
};
