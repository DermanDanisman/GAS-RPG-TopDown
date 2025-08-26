# Framed Value Widget

Last updated: 2025-08-25

## Goal

Create a reusable framed value widget (WP_FramedValue) that displays attribute values with a customizable background frame. This widget serves as a building block for the Attribute Menu system.

## Prerequisites

- Basic UMG (Unreal Motion Graphics) knowledge
- Understanding of UE5 Widget Blueprints
- Familiarity with the project's CoreUserWidget base class (OraUserWidget equivalent)

## Setup Instructions

### 1. Create Content Folder Structure

1. In the Content Browser, navigate to the UI folder
2. Create a new folder called "Attribute Menu"
3. This will serve as the container for all attribute menu-related widgets

### 2. Create the Base Widget Blueprint

1. In the "Attribute Menu" folder, create a new Widget Blueprint
2. Name it `WP_FramedValue` 
3. Set the parent class to `CoreUserWidget` (the project's equivalent of OraUserWidget)

### 3. Configure Root Widget - SizeBox

1. In the Widget Designer, replace the default Canvas Panel with a **SizeBox**
2. Set SizeBox properties:
   - **Size to Content**: false
   - **Override Desired Size**: true
   - **Width Override**: 200 (this will become a variable)
   - **Height Override**: 50 (this will become a variable)

#### Create Variables for Size Control

1. Create variables:
   - `BoxWidth` (float, default: 200.0)
   - `BoxHeight` (float, default: 50.0)
2. Mark both variables as **Instance Editable** and **BlueprintReadWrite**

#### Implement PreConstruct Function

1. Override the **PreConstruct** function
2. Create a custom function called `UpdateFrameSize`
3. In `UpdateFrameSize`:
   - Get reference to the SizeBox
   - Set Width Override to `BoxWidth` variable
   - Set Height Override to `BoxHeight` variable
4. Call `UpdateFrameSize` from PreConstruct

### 4. Add Overlay Container

1. Add an **Overlay** widget as a child of the SizeBox
2. Set Overlay **Slot** properties:
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

### 5. Background Image Setup

1. Add an **Image** widget as the first child of the Overlay
2. Name it "BackgroundImage"
3. Configure the background:
   - **Brush**: Set to material instance `MI_Flowing_UI_BG` (if available)
   - **Tint**: White (default, will be customizable)
   - **Slot Alignment**: Fill both horizontal and vertical

#### Create Background Customization Variables

1. Create variable `BackgroundBrush` (type: Slate Brush Structure)
2. Mark as **Instance Editable** and **BlueprintReadWrite**
3. Create a custom function `UpdateBackgroundBrush`:
   - Get reference to BackgroundImage
   - Set Brush to `BackgroundBrush` variable
   - Apply any tint modifications

#### Background Tint Options

1. Add variables for tint customization:
   - `BackgroundTintColor` (Linear Color, default: White)
   - `EnableTintOverride` (Boolean, default: false)
2. Update `UpdateBackgroundBrush` to apply tint when enabled

### 6. Border Frame

1. Add a second **Image** widget to the Overlay (above background)
2. Name it "BorderFrame" 
3. Configure border properties:
   - **Brush**: Set to `Border_1.png` (or equivalent border asset)
   - **Draw As**: Border (not Image)
   - **Margin**: 0.5 (uniform margin for proper border scaling)
   - **Slot Alignment**: Fill both horizontal and vertical

### 7. Text Display

1. Add a **Text Block** widget as the top child of the Overlay
2. Name it `TextBlock_Value`
3. Configure text properties:
   - **Font**: Choose appropriate game font (e.g., project's main UI font)
   - **Size**: 24 (or appropriate size for your UI scale)
   - **Justification**: Center
   - **Color**: White or appropriate contrast color
4. Add text outline for better readability:
   - **Shadow & Outline → Enable Outline**: true
   - **Outline Color**: Black or dark color
   - **Outline Size**: 1-2 pixels

#### Configure Text Alignment

1. Set Text Block **Slot** properties:
   - **Horizontal Alignment**: Center
   - **Vertical Alignment**: Center

### 8. Essential Variables

Create and configure these key variables:

1. `BoxWidth` (float, Instance Editable): Controls widget width
2. `BoxHeight` (float, Instance Editable): Controls widget height  
3. `BackgroundBrush` (Slate Brush, Instance Editable): Customizable background
4. `TextBlock_Value` (Text Block Reference): Mark as **Is Variable** for external access

#### Variable Visibility Settings

- Mark `TextBlock_Value` as **Is Variable** to allow external widgets to access and modify the text
- Consider making `BoxWidth` and `BoxHeight` **Instance Editable** for easy tweaking in child blueprints

## Usage Notes

### Live Tweaking

- The `BoxWidth` and `BoxHeight` variables allow for real-time size adjustments
- Background brush can be swapped per instance for different visual themes
- Text content should be set externally by parent widgets or controllers

### Child Blueprint Overrides

- Child blueprints can inherit from WP_FramedValue
- Override `BackgroundBrush` in child classes for specialized themes
- The `UpdateBackgroundBrush` function can be extended for more complex visual effects

### Integration with Widget Controller

- This widget is designed to work with the project's Widget Controller pattern
- External controllers should populate the `TextBlock_Value` text content
- Consider binding to controller delegates for real-time attribute updates

## Next Steps

Once the Framed Value widget is complete, proceed to create the composite [Text Value Row](text-value-row.md) widget that combines multiple framed values for attribute display.

## Troubleshooting

### Common Issues

- **Size not updating**: Ensure `UpdateFrameSize` is called in PreConstruct
- **Background not showing**: Verify the material instance `MI_Flowing_UI_BG` exists and is assigned
- **Border not displaying correctly**: Check that `Border_1.png` is set to "Draw As: Border" mode
- **Text alignment issues**: Confirm Text Block slot alignment is set to Center/Center

### Performance Considerations

- The background material instance should be lightweight for UI use
- Consider texture streaming settings for UI assets
- Test widget scaling across different screen resolutions