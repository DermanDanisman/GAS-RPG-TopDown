# Text Value Button Row Widget

Last updated: 2025-08-26

## Goal

Create a specialized text value row widget (WP_TextValueButtonRow) that extends WP_TextValueRow by adding an interactive button via the NamedSlot. This widget displays attribute labels with their corresponding values alongside a clickable "+" button, commonly used for attribute point spending in RPG attribute menu systems.

## Prerequisites

- Basic UMG (Unreal Motion Graphics) knowledge
- Understanding of UE5 Widget Blueprints
- Familiarity with the project's CoreUserWidget base class (OraUserWidget equivalent)
- Completion of the [Framed Value](framed-value.md) widget setup
- Completion of the [Text Value Row](text-value-row.md) widget setup

## Setup Instructions

### 1. Create the Widget Blueprint

1. In the Content Browser, navigate to **UI/Attribute Menu**
2. Create a new Widget Blueprint
3. Name it `WP_TextValueButtonRow`
4. Set the parent class to `WP_TextValueRow` (inherits from CoreUserWidget)

### 2. Configure the NamedSlot Content

The base WP_TextValueRow widget already includes a NamedSlot called `NamedSlot_Additional`. We'll populate this slot with button content.

1. In the Widget Designer, locate the **NamedSlot_Additional** in the hierarchy
2. The NamedSlot should already be configured with:
   - **Horizontal Alignment**: Right
   - **Vertical Alignment**: Center
   - **Size**: Auto

### 3. Add Overlay Container

1. Add an **Overlay** as a child of the NamedSlot_Additional
2. Name it "ButtonOverlay"
3. Set Overlay properties:
   - **Horizontal Alignment**: Center
   - **Vertical Alignment**: Center
   - **Size**: Auto

### 4. Configure Button Background Image

1. Add an **Image** widget as the first child of the Overlay
2. Name it "Image_Border"
3. Configure the background image:
   - **Brush**: Search for "button" assets and choose an appropriate button border asset
   - **Draw As**: Image (not Border or Rounded Box)
   - **Desired Size**: Set to approximately 40x40 (or 50x50 depending on design preference)
   - **Horizontal Alignment**: Center
   - **Vertical Alignment**: Center

**Note**: The actual size should be constrained by the parent SizeBox to avoid exceeding bounds.

### 5. Add Interactive Button

1. Add a **Button** widget as the second child of the Overlay
2. Name it "Button_Main"
3. Position it centered on top of the Image_Border
4. Configure Button styling using project assets:
   - **Style → Normal**: Use "Button" asset, Draw As = Image, Image Size ~40x40, center aligned
   - **Style → Hovered**: Use "Button_Highlighted" asset, Draw As = Image, Image Size ~40x40, center aligned
   - **Style → Pressed**: Use "Button_Pressed" asset, Draw As = Image, Image Size ~40x40, center aligned
   - **Style → Disabled**: Use "Button_Grayed" asset, Draw As = Image, Image Size ~40x40, center aligned

**Important**: Set Draw As = Image for all styles to avoid rounded corners.

### 6. Add Button Text

1. Add a **Text** widget as the third child of the Overlay
2. Name it "TextBlock_ButtonLabel"
3. Configure the text:
   - **Text**: "+"
   - **Horizontal Alignment**: Center
   - **Vertical Alignment**: Center
   - **Justification**: Center (both horizontal and vertical)
   - **Font**: Choose from project fonts (Roboto/Pirata/Amaranth examples)
   - **Outline**: Size 1 for better visibility
4. Position the text centered on top of the button

### 7. Essential Variables

Create and configure these key variables in addition to inherited ones:

1. `Button_Main` (Button Reference): Mark as **Is Variable** for external access to button events
2. `TextBlock_ButtonLabel` (Text Block Reference): Mark as **Is Variable** for text customization
3. `Image_Border` (Image Reference): Mark as **Is Variable** for background customization

#### Variable Visibility Settings

- Mark `Button_Main` as **Is Variable** to allow external access for click event binding
- Mark `TextBlock_ButtonLabel` as **Is Variable** to allow customization of button text
- Inherit all size and content variables from the parent WP_TextValueRow class

## Recommended Functions

### Inherited Functions

The widget inherits `UpdateLabel` and `UpdateValue` functions from WP_TextValueRow:

```cpp
// Inherited from WP_TextValueRow
void UpdateLabel(FText NewLabelText)
void UpdateValue(FText NewValueText)
```

### New Button-Specific Functions

### UpdateButtonText Function

Create a function to customize the button text:

```cpp
// Blueprint implementation
void UpdateButtonText(FText NewButtonText)
{
    if (TextBlock_ButtonLabel)
    {
        TextBlock_ButtonLabel->SetText(NewButtonText);
    }
}
```

### SetButtonEnabled Function

Create a function to control button interactivity:

```cpp
// Blueprint implementation
void SetButtonEnabled(bool bEnabled)
{
    if (Button_Main)
    {
        Button_Main->SetIsEnabled(bEnabled);
    }
}
```

## Event Handling

### Button Click Event

1. In the Widget Designer, select **Button_Main**
2. In the Details panel, find the **Events** section
3. Create an **OnClicked** event handler
4. In the Blueprint Graph, create a custom event or delegate to forward the click event

### Custom Delegate (Recommended)

Create a BlueprintAssignable delegate to expose button clicks to external controllers:

```cpp
// Blueprint implementation - create delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonClicked);

// Expose as BlueprintAssignable property
UPROPERTY(BlueprintAssignable)
FOnButtonClicked OnButtonClicked;
```

Bind the Button_Main OnClicked event to broadcast this delegate.

## Usage Notes

### Integration with Widget Controller

- This widget extends the Widget Controller pattern from its parent WP_TextValueRow
- External controllers should bind to the OnButtonClicked delegate for attribute point spending logic
- Use inherited functions to populate label and value content
- Consider binding button enabled state to available attribute points

### Size Constraints

- Keep button sizes reasonable (40x40 or 50x50 recommended) to avoid exceeding the enclosing SizeBox bounds
- Test with different BoxWidth values to ensure proper layout scaling
- The button should maintain proper alignment within the NamedSlot area

### Common Use Cases

- Attribute point spending: "Strength: 15 [+]"
- Skill upgrades: "Fireball Mastery: 3 [+]"
- Resource allocation: "Available Points: 5 [+]"

### Styling Consistency

- Use project-consistent button assets across all button states
- Maintain visual harmony with the overall attribute menu design
- Consider accessibility contrast ratios for button text and backgrounds

## Next Steps

Once the Text Value Button Row widget is complete, you can integrate multiple instances into the complete Attribute Menu system with proper data binding, controller integration, and attribute point spending flow.

## Troubleshooting

### Common Issues

- **Button not responding**: Verify Button_Main is marked as **Is Variable** and OnClicked event is properly bound
- **Button sizing issues**: Check that Image_Border and Button sizes are within SizeBox constraints
- **Text not visible**: Ensure TextBlock_ButtonLabel has proper outline settings and contrasting colors
- **Layout alignment problems**: Verify Overlay positioning and child widget alignments are set to Center

### Performance Considerations

- Reuse button style assets across multiple button row instances for better batching
- Consider button state caching to avoid unnecessary style updates
- Test button responsiveness across different screen resolutions and input methods

### Design Guidelines

- Maintain consistent button sizing across all attribute rows
- Use visual feedback (hover/pressed states) to improve user experience
- Ensure button hit area is large enough for touch input on mobile platforms