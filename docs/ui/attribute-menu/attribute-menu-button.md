# Attribute Menu Button Widget

Last updated: 2024-12-19

## Goal

Create and wire up the Attributes Button (WP_AttributeMenuButton) to open the Attribute Menu interface. This involves setting up the button widget, binding click events, managing button states to prevent spamming, positioning the menu appropriately, and implementing proper cleanup through an Event Dispatcher pattern.

## Prerequisites

- Basic UMG (Unreal Motion Graphics) knowledge
- Understanding of UE5 Widget Blueprints
- Familiarity with the project's CoreUserWidget base class (OraUserWidget equivalent)
- Completion of the [Attribute Menu Container](attribute-menu.md) widget setup

## Setup Instructions

### 1. Create the Widget Blueprint

1. In the Content Browser, navigate to **UI/Attribute Menu**
2. Create a new Widget Blueprint
3. Name it `WP_AttributeMenuButton`
4. Set the parent class to `CoreUserWidget` (the project's equivalent of OraUserWidget)

### 2. Configure the Button Widget

#### Widget Structure

1. Create a wide button wrapper containing an internal Button named "Button"
2. Configure the widget as a reusable component for opening the attribute menu
3. Mark the Attributes Button instance as **Is Variable** to allow external access

#### Essential Variables

Create and configure these key variables:

1. `Button` (Button Reference): Mark as **Is Variable** for external access to button events
2. Configure the button for attribute menu access functionality

### 3. Event Binding Setup

#### Event Construct Implementation

1. In the Widget Designer, go to the **Graph** tab
2. Override the **Event Construct** function
3. In Event Construct, implement the following:
   - Get reference to the internal Button
   - Bind the Button's **OnClicked** event to a custom event named `AttributeMenuButtonClicked`

### 4. AttributeMenuButtonClicked Event Implementation

Create a custom event called `AttributeMenuButtonClicked` and implement the following logic:

#### Step 1: Disable Button to Prevent Spamming

1. Get reference to the internal Button
2. Call **Set Is Enabled** with value `false`
3. This prevents users from spamming clicks while the menu is open

#### Step 2: Create and Display the Attribute Menu

1. Create Widget: **WP_AttributeMenu**
2. Use **Get Player Controller 0** as the owning player
3. Call **Add to Viewport** on the created menu widget

#### Step 3: Set Menu Position

1. After adding to viewport, call **Set Position in Viewport** on the menu widget
2. Use example offset coordinates such as (50, 50) or (25, 25)
3. Adjust positioning based on your UI layout requirements

> **Note:** Test different offset values to find the optimal positioning for your specific UI design.

### 5. Menu Size and Layout Fixes

#### Full Viewport Issue

If the menu fills the whole viewport and overlaps other UI elements:

1. Follow the detailed instructions in [attribute-menu.md](attribute-menu.md) to fix the issue
2. Wrap the menu's root SizeBox in an Overlay container
3. Re-test the positioning after implementing the fix

#### Background Padding Adjustment

If border overlaps are visible on the menu:

1. Adjust the background padding on the menu's background image
2. Change padding from the default value (e.g., 1) to a higher value (e.g., 3–5 or 25)
3. Test different padding values until border overlaps are eliminated

### 6. Visual Testing and Adjustments

#### Button Positioning

1. It's acceptable to lower or offset the attributes button position
2. This allows you to see the button in its disabled state while the menu is open
3. Provides visual feedback that the button interaction is temporarily disabled

#### Position Testing

1. Test the menu positioning with your specific UI layout
2. Ensure the menu doesn't overlap critical UI elements
3. Verify that the offset coordinates work well across different screen resolutions

## Usage Notes

### Integration Pattern

- This widget implements a standard open-menu button pattern
- The button disabling mechanism prevents multiple menu instances
- Position settings should be coordinated with your overall UI layout

### State Management

- Button is disabled immediately when clicked to prevent spam
- Button remains disabled while the menu is open
- Re-enabling is handled through the Event Dispatcher pattern (see attribute-menu.md)

### Performance Considerations

- Menu widgets are created fresh on each button click
- This pattern avoids hidden widgets reacting to callbacks
- Memory cleanup is handled through the destroy/remove pattern

## Next Steps

- **Event Dispatcher Integration**: Implement the dispatcher pattern for proper button re-enabling (detailed in [attribute-menu.md](attribute-menu.md))
- **Positioning Refinement**: Fine-tune menu positioning based on final UI design requirements
- **State Persistence**: Consider whether menu state should persist between opens or reset each time
- **Accessibility**: Ensure button states are properly communicated for accessibility compliance

This button widget serves as the entry point for the attribute management system and should integrate seamlessly with the overall UI architecture.