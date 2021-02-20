/*
  ==============================================================================

    LWSLookAndFeel.h
    Created: 5 Feb 2021 5:31:20pm
    Author:  Filipe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>



class LWSLookAndFeel
{

public:

#define greyColour Colour::fromRGBA(85, 85, 85, 100);
#define greenColour Colour(7, 165, 18);
#define blueColour Colour::fromRGBA(7, 112, 165, 100);
#define orangeColour Colour::fromRGBA(165, 71, 7, 100);
#define whiteColour Colours::whitesmoke;
#define redColour Colour::fromRGBA(156, 26, 21, 100);

    //Square Button
    class buttonLookAndFeel : public LookAndFeel_V4
    {
    public:
        buttonLookAndFeel() : LookAndFeel_V4()
        {

        }

        void drawTickBox(Graphics& g, Component& component,
            float x, float y, float w, float h,
            const bool ticked,
            const bool isEnabled,
            const bool shouldDrawButtonAsHighlighted,
            const bool shouldDrawButtonAsDown) override
        {
            ignoreUnused(isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            Rectangle<float> tickBounds(x, y, w, h);

            g.setColour(component.findColour(ToggleButton::tickDisabledColourId));
            g.drawRect(tickBounds, 1.0f);

            if (ticked)
            {
                g.setColour(component.findColour(ToggleButton::tickColourId));
                //auto tick = getCrossShape(1.f);
                g.fillRect(tickBounds.reduced(2.f, 2.f));
                //g.fillPath(tick, tick.getTransformToScaleToFit(tickBounds.reduced(4, 4).toFloat(), false));
            }
        };

    };

    //==========================================================================================

    //VU Meters
    class MeterLookAndFeel : public LookAndFeel_V4
    {
    public:

        MeterLookAndFeel() : LookAndFeel_V4()
        {

        }

        virtual void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
            float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override

        {
            double pi = MathConstants<double>::pi;
            rotaryStartAngle = (1.5 * pi) + ((5 * pi) / 18);
            rotaryEndAngle = rotaryStartAngle + 0.44444444444 * pi; //+80º 
            sliderPos = jlimit<float>(0.f, 1.f, meterSample);
            auto outline = Colours::whitesmoke;
            auto attackColor = Colour(7, 165, 18);
            auto sustainColor = Colour(7, 112, 165);
            auto redColor = Colour(156, 26, 21);

            auto bounds = Rectangle<int>(x, y, width, height).toFloat();

            auto radius = jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
            auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto lineW = jmin(8.0f, radius * 0.5f);
            auto arcRadius = radius * 1.5 - (lineW * 0.5f);

            //Fill meter background
            g.setColour(Colours::black);
            g.fillEllipse(bounds.reduced(2));

            Path arcOutlineWhite;
            //outside arc
            arcOutlineWhite.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY() * 2,
                arcRadius,
                arcRadius,
                0.0f,
                rotaryStartAngle,
                rotaryEndAngle - ((25 * pi) / 180),
                true);

            g.setColour(outline);
            g.setOpacity(0.75);
            g.strokePath(arcOutlineWhite, PathStrokeType(3, PathStrokeType::curved, PathStrokeType::rounded));
            

            //red arc
            Path arcOutlineRed;
            arcOutlineRed.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY() * 2,
                arcRadius,
                arcRadius,
                0.0f,
                rotaryEndAngle - ((25 * pi) / 180),
                rotaryEndAngle,
                true);

            g.setColour(redColor);
            g.setOpacity(0.75);
            g.strokePath(arcOutlineRed, PathStrokeType(3, PathStrokeType::curved, PathStrokeType::rounded));



            //draw L or R text in meters
            {
                g.setFont(bounds.getHeight() / 7);
                g.setColour(Colours::darkgrey);
                g.drawText(meterName, bounds, Justification::centred);
            }

            //draw LWS branding on meters
            {
                g.setFont(bounds.getHeight() / 10);
                g.setColour(Colour(165, 71, 7));
                g.setOpacity(0.5f);
                auto y = bounds.getY() + bounds.getHeight();
                Rectangle<float> rect;
                rect.setBounds(bounds.getX(), y - (bounds.getHeight() / 5), bounds.getWidth(), bounds.getHeight() / 5);
                g.drawText(lwsBrand, rect, Justification::centred);
            }

            if (slider.isEnabled())
            {
                //fill with color

                Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                    bounds.getCentreY() * 2,
                    arcRadius,
                    arcRadius,
                    0.0f,
                    rotaryStartAngle,
                    toAngle,
                    true);

                Path smallerArc;
                smallerArc.addCentredArc(bounds.getCentreX(),
                    bounds.getCentreY() * 2,
                    arcRadius - (arcRadius * 0.6),
                    arcRadius - (arcRadius * 0.6),
                    0.0f,
                    rotaryStartAngle,
                    toAngle,
                    true);

                g.setColour(attackColor);

                //g.strokePath(valueArc, PathStrokeType(lineW * 2, PathStrokeType::curved, PathStrokeType::butt));
                auto point = valueArc.getCurrentPosition();
                auto startNeedlepoint = smallerArc.getCurrentPosition();
                startNeedlepoint.setY(bounds.getCentreY() * 1.65);

                //g.drawLine(startNeedlepoint.getX(), startNeedlepoint.getY(), point.getX(), point.getY() + 3, lineW * 0.25);
                Line<float> needle;
                needle.setStart(startNeedlepoint.getX(), startNeedlepoint.getY());
                needle.setEnd(point.getX(), point.getY() + 3);
                g.drawArrow(needle, lineW * 0.25, lineW, lineW);
            }

            auto thumbWidth = lineW * 2.0f;
            Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
                bounds.getCentreY() + arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));

            g.setColour(Colours::whitesmoke);
            if (sliderPos == 0.5) g.fillRect(Rectangle<float>(1, thumbWidth).withCentre(thumbPoint));

            g.setColour(Colours::darkgrey);

            g.drawEllipse(bounds.reduced(2), 1); //outer ellipse
            g.drawEllipse(bounds.reduced(6), 1); //inner ellipse
            g.drawHorizontalLine(bounds.getCentreY() * 1.65, bounds.getWidth() * 0.17, bounds.getWidth() * 0.83);

        }

        String meterName{ "L" };
        String lwsBrand{ "LWS" };
        float meterSample{ 0.f };
    };

    //================================================================================

    //Linear Bar slider with inverted track colour painting
    class ReverseSliderLookAndFeel : public LookAndFeel_V4
    {
    public:

        ReverseSliderLookAndFeel() : LookAndFeel_V4()
        {

        }

        void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
            float sliderPos,
            float minSliderPos,
            float maxSliderPos,
            const Slider::SliderStyle style, Slider& slider) override
        {
            if (slider.isBar())
            {
                g.setColour(slider.findColour(Slider::trackColourId));
                Rectangle<float> fillRectangle;
                fillRectangle.setBounds(sliderPos, (float)y + 0.5f, (float)width - sliderPos, (float)height - 1.0f);
                g.fillRect(slider.isHorizontal() ? fillRectangle
                    : Rectangle<float>((float)x + 0.5f, sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos)));
            }
            else
            {
                auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
                auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

                auto trackWidth = jmin(6.0f, slider.isHorizontal() ? (float)height * 0.25f : (float)width * 0.25f);

                Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
                    slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

                Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
                    slider.isHorizontal() ? startPoint.y : (float)y);

                Path backgroundTrack;
                backgroundTrack.startNewSubPath(startPoint);
                backgroundTrack.lineTo(endPoint);
                g.setColour(slider.findColour(Slider::backgroundColourId));
                g.strokePath(backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

                Path valueTrack;
                Point<float> minPoint, maxPoint, thumbPoint;

                if (isTwoVal || isThreeVal)
                {
                    minPoint = { slider.isHorizontal() ? minSliderPos : (float)width * 0.5f,
                                 slider.isHorizontal() ? (float)height * 0.5f : minSliderPos };

                    if (isThreeVal)
                        thumbPoint = { slider.isHorizontal() ? sliderPos : (float)width * 0.5f,
                                       slider.isHorizontal() ? (float)height * 0.5f : sliderPos };

                    maxPoint = { slider.isHorizontal() ? maxSliderPos : (float)width * 0.5f,
                                 slider.isHorizontal() ? (float)height * 0.5f : maxSliderPos };
                }
                else
                {
                    auto kx = slider.isHorizontal() ? sliderPos : ((float)x + (float)width * 0.5f);
                    auto ky = slider.isHorizontal() ? ((float)y + (float)height * 0.5f) : sliderPos;

                    minPoint = startPoint;
                    maxPoint = { kx, ky };
                }

                auto thumbWidth = getSliderThumbRadius(slider);

                valueTrack.startNewSubPath(minPoint);
                valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
                g.setColour(slider.findColour(Slider::trackColourId));
                g.strokePath(valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

                if (!isTwoVal)
                {
                    g.setColour(slider.findColour(Slider::thumbColourId));
                    g.fillEllipse(Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
                }

                if (isTwoVal || isThreeVal)
                {
                    auto sr = jmin(trackWidth, (slider.isHorizontal() ? (float)height : (float)width) * 0.4f);
                    auto pointerColour = slider.findColour(Slider::thumbColourId);

                    if (slider.isHorizontal())
                    {
                        drawPointer(g, minSliderPos - sr,
                            jmax(0.0f, (float)y + (float)height * 0.5f - trackWidth * 2.0f),
                            trackWidth * 2.0f, pointerColour, 2);

                        drawPointer(g, maxSliderPos - trackWidth,
                            jmin((float)(y + height) - trackWidth * 2.0f, (float)y + (float)height * 0.5f),
                            trackWidth * 2.0f, pointerColour, 4);
                    }
                    else
                    {
                        drawPointer(g, jmax(0.0f, (float)x + (float)width * 0.5f - trackWidth * 2.0f),
                            minSliderPos - trackWidth,
                            trackWidth * 2.0f, pointerColour, 1);

                        drawPointer(g, jmin((float)(x + width) - trackWidth * 2.0f, (float)x + (float)width * 0.5f), maxSliderPos - sr,
                            trackWidth * 2.0f, pointerColour, 3);
                    }
                }
            }
        }
    };

    //============================================================

    //Custom non round ComboBox
    class LWSComboBox : public LookAndFeel_V4
    {
    public:
        void drawComboBox(Graphics& g, int width, int height, bool,
            int, int, int, int, ComboBox& box) override
        {
            Rectangle<int> boxBounds(0, 0, width, height);

            g.setColour(backgroundColour);
            g.fillRect(boxBounds.toFloat());

            g.setColour(outlineColour);
            g.drawRect(boxBounds.toFloat().reduced(0.5f, 0.5f), 1.0f);

            Rectangle<int> arrowZone(width - (width * 0.25), 0, width * 0.25, height);
            Path path;
            path.startNewSubPath((float)arrowZone.getX() + 3.0f, (float)arrowZone.getCentreY() - 2.0f);
            path.lineTo((float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 3.0f);
            path.lineTo((float)arrowZone.getRight() - 3.0f, (float)arrowZone.getCentreY() - 2.0f);

            g.setColour(arrowColour.withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
            g.strokePath(path, PathStrokeType(2.0f));
        }

        Colour backgroundColour = Colours::black;
        Colour outlineColour = Colours::darkgrey;
        Colour arrowColour = Colours::darkgrey;
        Colour textColour = Colours::whitesmoke;
        Colour popupColour = Colours::black;
        Colour popupHighlight = Colours::darkgrey;

        void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
            const bool isSeparator, const bool isActive,
            const bool isHighlighted, const bool isTicked,
            const bool hasSubMenu, const String& text,
            const String& shortcutKeyText,
            const Drawable* icon, const Colour* const textColourToUse) override
        {
            if (isSeparator)
            {
                auto r = area.reduced(5, 0);
                r.removeFromTop(roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));

                g.setColour(popupColour);
                g.fillRect(r.removeFromTop(1));
            }
            else
            {

                auto r = Rectangle<int>(area.getX() + 1, area.getY(), area.getWidth() - 2, area.getHeight());

                if (isHighlighted && isActive)
                {
                    g.setColour(popupHighlight);
                    g.fillRect(r);

                    g.setColour(textColour.withAlpha(0.75f));
                }
                else
                {
                    g.setColour(popupColour);
                    g.fillRect(r);
                    g.setColour(textColour.withAlpha(0.75f));
                }

                r.reduce(jmin(5, area.getWidth() / 20), 0);

                auto font = getPopupMenuFont();

                auto maxFontHeight = (float)r.getHeight() / 1.3f;

                if (font.getHeight() > maxFontHeight)
                    font.setHeight(maxFontHeight);

                g.setFont(font);

                auto iconArea = r.removeFromLeft(roundToInt(maxFontHeight)).toFloat();

                if (icon != nullptr)
                {
                    icon->drawWithin(g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
                    r.removeFromLeft(roundToInt(maxFontHeight * 0.5f));
                }
                else if (isTicked)
                {
                    auto tick = getTickShape(1.0f);
                    g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
                }

                if (hasSubMenu)
                {
                    auto arrowH = 0.6f * getPopupMenuFont().getAscent();

                    auto x = static_cast<float> (r.removeFromRight((int)arrowH).getX());
                    auto halfH = static_cast<float> (r.getCentreY());

                    Path path;
                    path.startNewSubPath(x, halfH - arrowH * 0.5f);
                    path.lineTo(x + arrowH * 0.6f, halfH);
                    path.lineTo(x, halfH + arrowH * 0.5f);

                    g.strokePath(path, PathStrokeType(2.0f));
                }

                r.removeFromRight(3);
                g.drawFittedText(text, r, Justification::centredLeft, 1);

                if (shortcutKeyText.isNotEmpty())
                {
                    auto f2 = font;
                    f2.setHeight(f2.getHeight() * 0.75f);
                    f2.setHorizontalScale(0.95f);
                    g.setFont(f2);

                    g.drawText(shortcutKeyText, r, Justification::centredRight, true);
                }
            }
        }
    };

    class RotaryLookAndFeel : public LookAndFeel_V4
    {
    public:

        RotaryLookAndFeel() : LookAndFeel_V4()
        {

        }

        virtual void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
            const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override

        {

            auto outline = Colours::whitesmoke;
            auto attackColor = Colour::fromRGBA(7, 165, 18, 80);
            auto sustainColor = Colour(7, 112, 165);

            auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

            auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto lineW = jmin(8.0f, radius * 0.5f);
            auto arcRadius = radius - lineW * 0.5f;

            Path backgroundArc;
            backgroundArc.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY(),
                arcRadius,
                arcRadius,
                0.0f,
                rotaryStartAngle,
                rotaryEndAngle,
                true);

            Path arcOutline;
            //outside arc
            arcOutline.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY(),
                arcRadius + lineW,
                arcRadius + lineW,
                0.0f,
                rotaryStartAngle,
                rotaryEndAngle,
                true);

            //inside arc
            arcOutline.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY(),
                arcRadius ,
                arcRadius ,
                0.0f,
                rotaryEndAngle,
                rotaryStartAngle,
                false);

            arcOutline.closeSubPath();


            g.setColour(outline);
            g.setOpacity(0.75);
            g.strokePath(arcOutline, PathStrokeType(1, PathStrokeType::curved, PathStrokeType::rounded));


            if (slider.isEnabled())
            {
                
                Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                    bounds.getCentreY(),
                    arcRadius + lineW * 0.5,
                    arcRadius + lineW * 0.5,
                    0.0f,
                    rotaryStartAngle,
                    toAngle,
                    true);

                g.setColour(attackColor);

                g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::butt));

                String value = std::to_string(static_cast<int>(sliderPos * 100)) + "%";
                g.setColour(Colours::whitesmoke);
                g.setFont(bounds.getHeight() * 0.3);
                g.drawText(value, bounds.reduced(5), Justification::centred);
                

            }

        }

    };
};
