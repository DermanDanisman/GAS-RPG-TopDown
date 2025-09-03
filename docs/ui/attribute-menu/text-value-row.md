# Text Value Row Widget

Last updated: 2024-12-19

## Goal

Create a reusable text value row widget (WP_TextValueRow) that displays attribute labels with their corresponding values in a horizontal layout. This widget combines text labels with framed values to create composite attribute displays (e.g., "Strength: 15") and serves as a building block for the Attribute Menu system.

## Prerequisites

- Basic UMG (Unreal Motion Graphics) knowledge
- Understanding of UE5 Widget Blueprints
- Familiarity with the project's CoreUserWidget base class (OraUserWidget equivalent)
- Completion of the [Framed Value](framed-value.md) widget setup

## Setup Instructions

### 1. Create the Widget Blueprint

1. In the Content Browser, navigate to **UI/Attribute Menu**
2. Create a new Widget Blueprint
3. Name it `WP_TextValueRow`
4. Set the parent class to `CoreUserWidget` (the project's equivalent of OraUserWidget)

### 2. Configure Root Widget - SizeBox

1. In the Widget Designer, replace the default Canvas Panel with a **SizeBox**
2. Set SizeBox properties:
   - **Size to Content**: false
   - **Override Desired Size**: true
   - **Width Override**: 600 (this will become a variable)
   - **Height Override**: 60 (this will become a variable)

#### Create Variables for Size Control

1. Create variables:
   - `BoxWidth` (float, default: 600.0, Instance Editable)
   - `BoxHeight` (float, default: 60.0, Instance Editable)

#### Implement PreConstruct Function

1. Override the **PreConstruct** function
2. Create a custom function called `UpdateFrameSize`
3. In `UpdateFrameSize`:
   - Get reference to the SizeBox
   - Set Width Override to `BoxWidth` variable
   - Set Height Override to `BoxHeight` variable
4. Call `UpdateFrameSize` from PreConstruct

### 3. Add HorizontalBox Layout

1. Add a **HorizontalBox** as a child of the SizeBox
2. Name it `HorizontalBox_Main`
3. Set HorizontalBox slot properties:
   - **Horizontal Alignment**: Fill
   - **Vertical Alignment**: Fill

### 4. Configure Left Text Label

1. Add a **Text Block** as the first child of the HorizontalBox
2. Name it `TextBlock_Label`
3. Configure text properties:
   - **Font**: Pirata One (or your game's main UI font)
   - **Size**: 32 (approximately)
   - **Letter Spacing**: 176 (approximately)
   - **Justification**: Left
   - **Color**: White or appropriate theme color

#### Text Outline Settings

1. Configure text outline for better readability:
   - **Shadow & Outline → Enable Outline**: true
   - **Outline Color**: Black or dark color
   - **Outline Size**: 1 pixel

#### Text Block Slot Configuration

1. Set Text Block slot properties in HorizontalBox:
   - **Horizontal Alignment**: Left  
   - **Vertical Alignment**: Center
   - **Size**: Auto (let content determine width)

### 5. Add Spacer for Gap Control

1. Add a **Spacer** after the text label in the HorizontalBox
2. Name it `Spacer_Gap`
3. Set Spacer slot properties:
   - **Size**: Fixed
   - **Fixed Size**: 40-50 pixels (adjust as needed)

This spacer controls the gap between the label and the value display, allowing for consistent spacing regardless of label length.

### 6. Insert Framed Value Widget

1. Add the **WP_FramedValue** widget after the spacer
2. Name it `FramedValue_Main`
3. Configure the framed value alignment:
   - **Horizontal Alignment**: Center or Right (as desired)
   - **Vertical Alignment**: Center

#### Alignment Tips for Framed Value

- **Center alignment**: Respects the framed value's own size, good for consistent appearance
- **Right alignment**: Maintains consistent distance to any following elements (like NamedSlot)

### 7. Add NamedSlot for Extensibility

1. Add a **NamedSlot** as the final child of the HorizontalBox
2. Name it `NamedSlot_Additional`
3. Set NamedSlot properties:
   - **Horizontal Alignment**: Right
   - **Vertical Alignment**: Center
   - **Size**: Auto

This allows child widgets (e.g., buttons, icons) to be injected into derived widgets or specific instances.

### 8. Handle Long Labels

For longer attribute names (e.g., "Critical Hit Resistance"):

1. Increase `BoxWidth` variable default to 800 or higher
2. Consider adjusting the spacer size for proper proportions
3. Test with the longest expected label text

### 9. Essential Variables

Create and configure these key variables:

1. `BoxWidth` (float, Instance Editable): Controls widget width
2. `BoxHeight` (float, Instance Editable): Controls widget height
3. `TextBlock_Label` (Text Block Reference): Mark as **Is Variable** for external access
4. `FramedValue_Main` (WP_FramedValue Reference): Mark as **Is Variable** for value updates

#### Variable Visibility Settings

- Mark `TextBlock_Label` as **Is Variable** to allow external access for label updates
- Mark `FramedValue_Main` as **Is Variable** to allow external access for value updates
- Consider making size variables **Instance Editable** for easy tweaking per use case

## Recommended Functions

### UpdateLabel Function

Create a function to update the label text:

```cpp
// Blueprint implementation
void UpdateLabel(FText NewLabelText)
{
    if (TextBlock_Label)
    {
        TextBlock_Label->SetText(NewLabelText);
    }
}
```

### UpdateValue Function

Create a function to update the displayed value:

```cpp
// Blueprint implementation  
void UpdateValue(FText NewValueText)
{
    if (FramedValue_Main && FramedValue_Main->TextBlock_Value)
    {
        FramedValue_Main->TextBlock_Value->SetText(NewValueText);
    }
}
```

## Usage Notes

### Integration with Widget Controller

- This widget is designed to work with the project's Widget Controller pattern
- External controllers should populate both label and value content
- Consider binding to controller delegates for real-time attribute updates

### Layout Flexibility

- The spacer allows precise control over label-to-value spacing
- NamedSlot enables adding custom elements (buttons, icons) in derived widgets
- Size variables allow per-instance customization

### Common Use Cases

- Primary attributes: "Strength: 15"
- Secondary attributes: "Attack Power: 142" 
- Resistances: "Fire Resistance: 25%"
- Long labels: "Critical Hit Resistance: 12%"

## Next Steps

Once the Text Value Row widget is complete, you can proceed to assemble multiple instances into the complete Attribute Menu system with proper data binding and controller integration.

## Troubleshooting

### Common Issues

- **Text not displaying**: Ensure TextBlock_Label is marked as **Is Variable** and has default text set
- **Value not updating**: Verify FramedValue_Main reference is valid and marked as **Is Variable**
- **Layout spacing issues**: Adjust spacer size and HorizontalBox slot alignments
- **Long labels overlapping**: Increase BoxWidth variable and test with longest expected text

### Performance Considerations

- Use the same font and material instances across multiple text value rows for better batching
- Consider object pooling for dynamic attribute lists
- Test scaling across different screen resolutions