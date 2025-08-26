# WP_Button Widget

Last updated: 2025-12-19

## Goal

Create a reusable button widget (WP_Button) that serves as a standardized interactive button component throughout the project. This widget provides a consistent visual design with configurable text, stylable states (normal, hovered, pressed, disabled), and customizable dimensions, eliminating the need for ad-hoc button implementations.

## Prerequisites

- Basic UMG (Unreal Motion Graphics) knowledge
- Understanding of UE5 Widget Blueprints
- Familiarity with the project's CoreUserWidget base class (OraUserWidget equivalent)

## Setup Instructions

### 1. Create the Widget Blueprint

1. In the Content Browser, navigate to **UI/Button**
2. Create a new Widget Blueprint
3. Name it `WP_Button`
4. Set the parent class to `CoreUserWidget` (the project's equivalent of OraUserWidget)

### 2. Configure Root Widget - SizeBox

1. In the Widget Designer, replace the default Canvas Panel with a **SizeBox**
2. Name it `SizeBox_Root`
3. Set SizeBox properties:
   - **Size to Content**: false
   - **Override Desired Size**: true
   - **Width Override**: 40 (will be controlled by variable)
   - **Height Override**: 40 (will be controlled by variable)

#### Create Size Control Variables

1. Create variables:
   - `BoxWidth` (float, default: 40.0, Instance Editable)
   - `BoxHeight` (float, default: 40.0, Instance Editable)
2. Mark both variables as **Instance Editable** and **BlueprintReadWrite**

#### Implement PreConstruct Function

1. Override the **PreConstruct** function
2. Create a custom function called `UpdateBoxSize`
3. In `UpdateBoxSize`:
   - Get reference to SizeBox_Root
   - Set Width Override to `BoxWidth` variable
   - Set Height Override to `BoxHeight` variable
4. Call `UpdateBoxSize` from PreConstruct

### 3. Add Main Overlay Container

1. Add an **Overlay** as a child of SizeBox_Root
2. Name it `Overlay_Root`
3. Set Overlay slot properties:
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

### 4. Configure Border Image

1. Add an **Image** as the first child of Overlay_Root
2. Name it `Image_Border`
3. Create a variable for the border:
   - `BorderBrush` (Slate Brush, Instance Editable, default: Button_Border asset)
4. Configure image properties:
   - **Brush**: Bind to `BorderBrush` variable
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

#### Implement Border Update Function

1. Create a custom function called `UpdateBorderBrush`
2. In `UpdateBorderBrush`:
   - Get reference to Image_Border
   - Set Brush to `BorderBrush` variable
3. Call `UpdateBorderBrush` from PreConstruct

### 5. Configure Interactive Button

1. Add a **Button** as the second child of Overlay_Root
2. Name it `Button_Main`
3. Mark as **Is Variable** for external access to click events
4. Set Button slot properties:
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

#### Create Button State Variables

Create variables under "Button Properties" category:

1. `ButtonNormalBrush` (Slate Brush, Instance Editable)
   - Default: Button asset
2. `ButtonHoveredBrush` (Slate Brush, Instance Editable)
   - Default: Button_Highlighted asset
3. `ButtonPressedBrush` (Slate Brush, Instance Editable)
   - Default: Button_Pressed asset
4. `ButtonDisabledBrush` (Slate Brush, Instance Editable)
   - Default: Button_Grayed asset

#### Configure Button Style

1. In Button properties, expand **Style** section
2. For each state (Normal, Hovered, Pressed, Disabled):
   - Set **Draw As**: Image (to avoid rounded corners)
   - Bind **Brush** to corresponding variable
3. Configure button behavior:
   - **Click Method**: Precise Click
   - **Touch Method**: Precise Tap

#### Implement Button Style Update Function

1. Create a custom function called `UpdateButtonBrushes`
2. In `UpdateButtonBrushes`:
   - Get reference to Button_Main
   - Apply style settings for all button states
   - Set Normal brush to `ButtonNormalBrush`
   - Set Hovered brush to `ButtonHoveredBrush`
   - Set Pressed brush to `ButtonPressedBrush`
   - Set Disabled brush to `ButtonDisabledBrush`
3. Call `UpdateButtonBrushes` from PreConstruct

### 6. Add Button Text

1. Add a **TextBlock** as the third child of Overlay_Root
2. Name it `TextBlock_ButtonText`
3. Configure text properties:
   - **Horizontal Alignment**: Center
   - **Vertical Alignment**: Center
   - **Text**: "X" (default value)

#### Create Text Configuration Variables

1. `ButtonText` (Text, Instance Editable, default: "X")
2. `FontFamily` (Font Family, Instance Editable, example: Roboto from Engine Content)
3. `FontSize` (int32, Instance Editable, default: 16)
4. `OutlineSize` (int32, Instance Editable, default: 1)
5. `LetterSpacing` (float, Instance Editable, optional)

#### Implement Text Update Function

1. Create a custom function called `UpdateText`
2. In `UpdateText`:
   - Get reference to TextBlock_ButtonText
   - Set Text to `ButtonText` variable
   - Apply font configuration:
     - Set Font Family to `FontFamily`
     - Set Font Size to `FontSize`
     - Set Outline Size to `OutlineSize`
     - Set Letter Spacing to `LetterSpacing` (if used)
3. Call `UpdateText` from PreConstruct

### 7. Essential Variables Summary

Create and configure these key variables:

#### Size Control
- `BoxWidth` (float, Instance Editable): Controls button width
- `BoxHeight` (float, Instance Editable): Controls button height

#### Visual Styling
- `BorderBrush` (Slate Brush, Instance Editable): Button border appearance
- `ButtonNormalBrush` (Slate Brush, Instance Editable): Normal state appearance
- `ButtonHoveredBrush` (Slate Brush, Instance Editable): Hover state appearance
- `ButtonPressedBrush` (Slate Brush, Instance Editable): Pressed state appearance
- `ButtonDisabledBrush` (Slate Brush, Instance Editable): Disabled state appearance

#### Text Configuration
- `ButtonText` (Text, Instance Editable): Display text
- `FontFamily` (Font Family, Instance Editable): Text font
- `FontSize` (int32, Instance Editable): Text size
- `OutlineSize` (int32, Instance Editable): Text outline
- `LetterSpacing` (float, Instance Editable): Character spacing

#### Widget References
- `Button_Main` (Button Reference): Mark as **Is Variable** for external click binding

## Usage Example

### Integration in WP_AttributeMenu

Replace the close button overlay implementation with a WP_Button instance:

1. In WP_AttributeMenu, locate the close button SizeBox in the bottom-right corner
2. Remove the existing Overlay + Image + Button setup
3. Add a **WP_Button** widget instance
4. Configure the WP_Button:
   - **ButtonText**: "X"
   - **BoxWidth**: 40
   - **BoxHeight**: 40
   - Override brushes or fonts if needed for specific styling
5. In your HUD/Widget Controller, bind to the WP_Button's `OnClicked` event

### External Click Handling

```cpp
// In Blueprint or C++, bind to the button's click event
WP_Button->Button_Main->OnClicked.AddDynamic(this, &UYourWidgetController::HandleCloseButtonClicked);
```

### Customization Examples

```cpp
// Example customization for different button types
// Close button (red theme)
CloseButton->ButtonText = FText::FromString("X");
CloseButton->BoxWidth = 40.0f;
CloseButton->BoxHeight = 40.0f;

// Action button (larger, different text)
ActionButton->ButtonText = FText::FromString("UPGRADE");
ActionButton->BoxWidth = 120.0f;
ActionButton->BoxHeight = 60.0f;
ActionButton->FontSize = 18;

// Small icon button
IconButton->ButtonText = FText::FromString("+");
IconButton->BoxWidth = 30.0f;
IconButton->BoxHeight = 30.0f;
IconButton->FontSize = 24;
```

## Configuration Notes

### Visual Consistency

- Use project-standard button assets across all state brushes
- Maintain visual harmony with overall UI design
- Consider accessibility contrast ratios for button text and backgrounds

### Font and Typography

- Font choices and spacing are design-owned decisions
- Values provided are examples and should be adjusted based on project requirements
- Consider readability across different screen sizes and resolutions

### State Management

- **Draw As: Image** prevents unwanted rounded corners from default button styling
- Each button state can use different assets for rich visual feedback
- Disabled state should provide clear visual indication of non-interactive state

## Advanced Usage

### Button State Control

```cpp
// Enable/disable button functionality
WP_Button->Button_Main->SetIsEnabled(false);

// Check current enabled state
bool bIsEnabled = WP_Button->Button_Main->GetIsEnabled();
```

### Dynamic Text Updates

```cpp
// Update button text at runtime
WP_Button->ButtonText = FText::FromString("New Text");
WP_Button->UpdateText();
```

### Custom Styling Override

```cpp
// Override specific state brushes at runtime
WP_Button->ButtonHoveredBrush = NewHoveredBrush;
WP_Button->UpdateButtonBrushes();
```

## Performance Considerations

- Button assets should be optimized for UI rendering
- Consider texture streaming settings for button state assets
- Minimize overdraw by using appropriately sized textures
- Test button responsiveness across different input methods (mouse, touch, gamepad)

## Troubleshooting

### Common Issues

- **Button not responding**: Verify `Button_Main` is marked as **Is Variable** and properly bound
- **Text not updating**: Ensure `UpdateText` is called after changing `ButtonText` variable
- **Styling not applying**: Check that `UpdateButtonBrushes` is called in PreConstruct
- **Size not correct**: Verify `UpdateBoxSize` is properly implemented and called

### Visual Issues

- **Rounded corners appearing**: Ensure all button states use **Draw As: Image**
- **Inconsistent sizing**: Check that all assets have consistent dimensions
- **Text alignment problems**: Verify TextBlock alignment settings are Center/Center
- **Border not showing**: Confirm `BorderBrush` asset exists and `UpdateBorderBrush` is called

## Next Steps

Once the WP_Button widget is complete, it can be used throughout the project for consistent button implementation:

- Replace ad-hoc button setups in existing widgets
- Use as a template for specialized button variants
- Integrate with Widget Controller pattern for centralized click handling
- Add accessibility features like keyboard navigation support

This reusable widget promotes UI consistency and reduces development time for future button implementations.