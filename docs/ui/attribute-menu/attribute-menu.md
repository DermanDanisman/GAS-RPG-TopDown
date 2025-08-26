# Attribute Menu Container Widget

Last updated: 2025-12-19

## Goal

Create the main attribute menu container widget (WP_AttributeMenu) that serves as the primary interface for displaying character attributes. This comprehensive widget combines multiple reusable components to create a complete attribute management screen with primary attributes, secondary attributes with scrolling, and interactive elements for attribute point allocation.

## Prerequisites

- Basic UMG (Unreal Motion Graphics) knowledge
- Understanding of UE5 Widget Blueprints
- Familiarity with the project's CoreUserWidget base class (OraUserWidget equivalent)
- Completion of the [Framed Value](framed-value.md) widget setup
- Completion of the [Text Value Row](text-value-row.md) widget setup
- Completion of the [Text Value Button Row](text-value-button-row.md) widget setup

> **Note:** Font choices and letter spacing values mentioned below are design decisions and examples only. These should be adjusted based on your project's visual design requirements.

## Setup Instructions

### 1. Create the Widget Blueprint

1. In the Content Browser, navigate to **UI/Attribute Menu**
2. Create a new Widget Blueprint
3. Name it `WP_AttributeMenu`
4. Set the parent class to `CoreUserWidget` (the project's equivalent of OraUserWidget)

### 2. Configure Root Widget - SizeBox

1. In the Widget Designer, replace the default Canvas Panel with a **SizeBox**
2. Name it `SizeBox_Root`
3. Set SizeBox properties:
   - **Size to Content**: false
   - **Override Desired Size**: true
   - **Width Override**: 805 (hardcoded for now, later adjustable)
   - **Height Override**: 960 (hardcoded for now, later adjustable)

> **Note:** Example dimensions are 805 x 960. These can be made variable later for different screen sizes or design requirements.

### 3. Add Main Overlay Container

1. Add an **Overlay** as a child of the SizeBox_Root
2. Name it `Overlay_Root`
3. Set Overlay slot properties:
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

### 4. Background Visuals Setup

#### Background Image

1. Add an **Image** as the first child of Overlay_Root
2. Name it `Image_Background`
3. Configure the background image:
   - **Brush**: Set to background material (e.g., `MI_Flowing_UI_BG`) with slight tint
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill
   - **Padding**: Set to 1 on all sides to avoid border overlap (optional)

#### Border Frame

1. Add another **Image** above the background as the second child of Overlay_Root
2. Name it `Image_Border`
3. Configure the border image:
   - **Brush**: Set to `Border_Large` asset
   - **Draw As**: Border
   - **Margins**: Set to 0.5 on all sides
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

### 5. Main Layout Container

1. Add a **WrapBox** as the third child of Overlay_Root
2. Name it `WrapBox_Root`
3. Configure WrapBox properties:
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill
   - **Padding**: Start with 25 on all sides (can be increased to 40 for tighter content)

> **Note:** When increasing padding to 40, reduce row widths accordingly (from 800 to 720-750).

### 6. Title and Subtitle Setup

#### Main Title

1. Add a **TextBlock** as the first child of WrapBox_Root
2. Name it `TextBlock_Title`
3. Configure title properties:
   - **Text**: "ATTRIBUTES"
   - **Fill Empty Space**: Enable this to allow centering
   - **Horizontal Alignment**: Center
   - **Font**: Example - Pirata One, size ~36, letter spacing ~400, outline 1 (design-owned values)

#### Force New Row Spacer

1. Add a **Spacer** after the title
2. Name it `Spacer_ForceNewRow`
3. Configure spacer:
   - **Fill Empty Space**: Disable
   - **Size X**: Set larger than container width (e.g., 1000 when container is ~805) to force next items to new row
   - **Size Y**: Keep minimal (e.g., 1)

> **Tip:** Use "Fill Span When Less Than" property and set it wider than your container to ensure proper row breaking.

#### Subtitle

1. Add another **TextBlock** after the spacer
2. Name it `TextBlock_Subtitle`
3. Configure subtitle properties:
   - **Text**: "PRIMARY ATTRIBUTES"
   - **Horizontal Alignment**: Center
   - **Font**: Example - size ~20, letter spacing ~800, outline 1 (design-owned values)

### 7. Spacing Control

Add **Spacer** widgets between major sections to control vertical spacing:

1. Add spacers with these properties:
   - **Size X**: Set wider than container (e.g., 750-800) so they occupy their own line
   - **Size Y**: Adjust for desired vertical spacing (start with ~20, later reduce to ~5-15 to optimize space)
2. Duplicate spacers as needed between sections

### 8. Attribute Points Row

1. Add a **WP_TextValueRow** widget before the PRIMARY ATTRIBUTES section
2. Configure the widget:
   - Set **BoxWidth** to fit WrapBox padding (750 with 25 padding, 720 with 40 padding)
   - Use for displaying "Attribute Points" available for spending
   - Position above the primary attributes section

### 9. Primary Attributes Section

1. Add multiple **WP_TextValueButtonRow** instances (typically 4 for primary attributes)
2. Configure each instance:
   - **BoxWidth**: Start with default 800, then tune to 750/720 based on WrapBox padding
   - The framed value should sit just left of a fixed 40px Spacer before the NamedSlot
   - Each row represents a primary attribute (e.g., Strength, Dexterity, Intelligence, Vitality)

### 10. Secondary Attributes Section

#### Section Header

1. Add a **TextBlock** for the secondary attributes header
2. Name it `TextBlock_SecondaryHeader`
3. Configure properties:
   - **Text**: "SECONDARY ATTRIBUTES"
   - **Horizontal Alignment**: Center
   - Same font style as subtitle (examples only)

#### Scrollable Container Setup

1. Add a **SizeBox** after the header
2. Name it `SizeBox_Scroll`
3. Configure SizeBox:
   - **Width Override**: Set to constrain the ScrollBox area
   - **Height Override**: Set to constrain the ScrollBox area
   - **Horizontal Alignment**: Center
   - **Vertical Alignment**: Center
   - **Fill Empty Space**: Optional, for row behavior

#### ScrollBox Implementation

1. Add a **ScrollBox** inside SizeBox_Scroll
2. Configure ScrollBox properties:
   - **Orientation**: Vertical
   - **Scroll Bar Visibility**: Auto (scrollbar appears when content exceeds fixed height)

#### Secondary Attribute Rows

1. Add multiple **WP_TextValueRow** widgets as children of the ScrollBox
2. Add approximately 10 rows for different secondary attributes
3. Each row displays read-only secondary attribute values (derived from primary attributes)

### 11. Close Button Setup

#### Button Container

1. Add a **SizeBox** as a child of Overlay_Root (positioned after WrapBox_Root)
2. Name it `SizeBox_CloseButton`
3. Configure SizeBox slot:
   - **Horizontal Alignment**: Right
   - **Vertical Alignment**: Bottom
   - **Width Override**: ~40
   - **Height Override**: ~40
   - **Padding**: Set right and bottom padding to ~25

#### Button Implementation

1. Add an **Overlay** inside the SizeBox
2. Add an **Image** for button border styling
3. Replace the ad-hoc button setup with a **WP_Button** widget instance:
   - **ButtonText**: "X"
   - **BoxWidth**: ~40
   - **BoxHeight**: ~40
   - Position it centered in the local overlay

> **Note:** The WP_Button widget should be used instead of manual button + image setup. See [WP_Button documentation](../button/wp-button.md) for details.

## Configuration Notes

### Size and Padding Guidelines

- **Default TextValue row width**: Start at 800 and reduce (e.g., 750 or 720) when WrapBox padding increases
- **Padding relationship**: 25px padding → 750px row width; 40px padding → 720px row width
- **Framed value alignment**: Keep right-aligned relative to trailing NamedSlot via fixed 40px spacer

### Font and Styling

- **Font families and sizes**: All values mentioned are examples only and design-owned
- **Letter spacing**: Values like 400, 800 are examples for dramatic effect
- **Outline settings**: Outline width of 1 is an example for text visibility

### Layout Behavior

- **WrapBox advantages**: Enables automatic row wrapping and centered title/subtitle layout
- **Fill Empty Space**: Use on title TextBlock to enable proper centering behavior
- **Spacer sizing**: Use spacers wider than container width to force new rows

## Usage Notes

### Integration with Widget Controller

- This widget is designed to work with the project's Widget Controller pattern
- External controllers should populate attribute values through the child WP_TextValueRow instances
- Button interactions should be bound through the Widget Controller for attribute point spending

### Responsive Design

- Container dimensions are initially hardcoded but can be made variable later
- Row widths should be adjusted proportionally when container padding changes
- Scrollable secondary attributes section handles overflow gracefully

### Performance Considerations

- ScrollBox only renders visible items, making it efficient for long attribute lists
- Background materials should be optimized for UI rendering
- Consider texture streaming settings for UI assets

## Open/Close + Dispatcher

### Close Button Wiring

To properly handle menu closing functionality:

#### Rename and Configure Close Button

1. In the close button setup (Section 11), rename the close button element to `CloseButton`
2. Mark `CloseButton` as **Is Variable** to allow external access

#### Event Construct Binding

1. Override **Event Construct** in the Widget Graph
2. Get reference to the CloseButton's internal Button component
3. Assign the Button's **OnClicked** event to call **RemoveFromParent**
4. This immediately closes the menu when the X button is clicked

### Event Dispatcher Implementation

To re-enable the Attributes Button without creating tight coupling between widgets:

#### Create Event Dispatcher

1. In WP_AttributeMenu, create a new **Event Dispatcher** named `AttributeMenuClosed`
2. Set the dispatcher as **BlueprintAssignable** to allow external binding

#### Event Destruct Implementation

1. Override **Event Destruct** in the Widget Graph
2. In Event Destruct, call **(Broadcast) AttributeMenuClosed**
3. This notifies any listening widgets when the menu is being destroyed

#### Attributes Button Re-enabling

In the overlay/HUD where WP_AttributeMenuButton lives:

1. Immediately after creating WP_AttributeMenu and setting its position
2. Bind to the menu's **AttributeMenuClosed** event dispatcher
3. In the callback function:
   - Get reference to WP_AttributeMenuButton
   - Get reference to its internal Button component
   - Call **Set Is Enabled** with value `true` on the internal Button

> **Important:** Call Set Is Enabled on the internal Button, not the wrapper widget.

### Alternative Pattern Note

You could keep a persistent menu instance and toggle visibility instead of creating/destroying the menu each time. Both approaches are acceptable, but this guide prefers the create/destroy pattern to avoid hidden widgets reacting to callbacks when they shouldn't be active.

## Next Steps

- **GAS Integration**: Wire up Gameplay Ability System bindings for real-time attribute updates
- **Button Interactions**: Implement attribute point spending logic through Widget Controller
- **Responsive Layout**: Convert hardcoded dimensions to variables for different screen sizes
- **Visual Polish**: Fine-tune fonts, spacing, and visual effects based on final design requirements

This widget serves as the foundation for the complete attribute management system and should be extended with gameplay logic in future development phases.