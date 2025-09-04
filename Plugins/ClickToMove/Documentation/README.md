# ClickToMove Plugin Documentation

Welcome to the complete documentation for the ClickToMove plugin for Unreal Engine 5. This documentation provides everything you need to integrate professional point-and-click movement into your project.

## 📚 Documentation Structure

### 🚀 [README](../README.md)
**Start here!** Overview of features, quick start guide, and basic usage examples.

**Contents:**
- Plugin overview and key features
- Quick installation and setup
- Basic integration examples
- Performance highlights
- License information

### 🔧 [Setup Guide](Setup-Guide.md)
**Complete installation and configuration instructions** from plugin installation to advanced integration scenarios.

**Contents:**
- Prerequisites and installation steps
- Project configuration (collision channels, navigation)
- Blueprint and C++ integration examples
- Enhanced Input system setup
- Multiplayer considerations
- Testing and verification

### 📖 [API Reference](API-Reference.md)
**Complete technical reference** for all classes, methods, and properties.

**Contents:**
- UClickToMoveComponent full API
- All public methods with parameters and usage
- Configuration properties and their effects
- Runtime state properties for debugging
- Blueprint and C++ integration patterns
- Performance considerations

### ⚙️ [Configuration Guide](Configuration-Guide.md)
**Detailed configuration options** to fine-tune movement behavior for your game.

**Contents:**
- Movement timing and thresholds
- Acceptance radius and speed scaling
- Pathfinding and lookahead settings
- Navigation projection configuration
- Debug and visualization options
- Game-specific configuration examples (Action RPG, Strategy, MMO, Mobile)

### 🔗 [Integration Examples](Integration-Examples.md)
**Practical examples** for integrating with common Unreal Engine systems.

**Contents:**
- Enhanced Input System integration
- Gameplay Ability System (GAS) coordination
- Custom interaction system examples
- Multiplayer networking patterns
- UI system integration
- Animation system coordination
- Custom component extensions

### 🐛 [Troubleshooting](Troubleshooting.md)
**Comprehensive troubleshooting guide** for common issues and their solutions.

**Contents:**
- Quick diagnostics checklist
- Movement issues (character not moving, not reaching destination)
- Performance problems and optimization
- Multiplayer synchronization issues
- Integration conflicts (GAS, Enhanced Input)
- Navigation and NavMesh problems
- Debug logging and profiling tools

## 🎯 Quick Navigation

### By User Type

**🎮 Game Designers / Blueprint Users**
1. [README](../README.md) - Get overview
2. [Setup Guide](Setup-Guide.md) - Blueprint integration section
3. [Configuration Guide](Configuration-Guide.md) - Tune for your game type

**👩‍💻 Programmers**
1. [README](../README.md) - Understand architecture
2. [Setup Guide](Setup-Guide.md) - C++ integration section
3. [API Reference](API-Reference.md) - Complete technical details
4. [Integration Examples](Integration-Examples.md) - Advanced patterns

**🛠️ Technical Artists**
1. [Configuration Guide](Configuration-Guide.md) - Visual and performance tuning
2. [Troubleshooting](Troubleshooting.md) - Debug visualization
3. [Integration Examples](Integration-Examples.md) - Animation integration

### By Task

**🆕 First Time Setup**
- [README](../README.md) → [Setup Guide](Setup-Guide.md)

**🔧 Adding to Existing Project**  
- [Setup Guide](Setup-Guide.md) → [Integration Examples](Integration-Examples.md)

**⚡ Performance Optimization**
- [Configuration Guide](Configuration-Guide.md) → [Troubleshooting](Troubleshooting.md)

**🐛 Fixing Issues**
- [Troubleshooting](Troubleshooting.md) → [API Reference](API-Reference.md)

**🎨 Customizing Behavior**
- [Configuration Guide](Configuration-Guide.md) → [Integration Examples](Integration-Examples.md)

## 🎮 Plugin Features Summary

### 🌟 Core Features
- **Dual Movement Modes**: Hold-to-move for direct control, short-press autorun for path following
- **Multiplayer Ready**: Client-side prediction with server replication via CharacterMovement
- **Advanced Pathfinding**: NavMesh integration with corner-cutting optimization
- **Framework Integration**: Works seamlessly with Enhanced Input and Gameplay Ability System

### ⚡ Performance Features
- **Conditional Ticking**: Only ticks during active autorun
- **Efficient Path Caching**: Reuses computed navigation paths
- **Local Controller Validation**: Prevents unnecessary execution on server/remote clients
- **Debug Exclusion**: All debug code excluded from shipping builds

### 🔧 Configuration Options
- **Movement Timing**: Configurable short-press threshold
- **Acceptance Radius**: Base radius with optional speed scaling
- **Pathfinding**: Lookahead blending for smooth corner cutting
- **Navigation**: Customizable projection extents and collision channels

## 🚀 Getting Started

### Minimal Setup (5 minutes)
1. Install plugin and enable in project
2. Add component to PlayerController
3. Bind mouse input to component functions
4. Add NavMeshBoundsVolume to level

See [Setup Guide](Setup-Guide.md) for detailed instructions.

### Advanced Integration
For complex projects with existing systems:
1. Review [Integration Examples](Integration-Examples.md) for your specific frameworks
2. Configure component using [Configuration Guide](Configuration-Guide.md)
3. Implement error handling using [Troubleshooting](Troubleshooting.md)

## 📋 Version Information

**Current Version**: 1.0.0  
**Engine Compatibility**: Unreal Engine 5.0+  
**Platform Support**: Windows, Mac, Linux, Android, iOS  
**Dependencies**: NavigationSystem (required), Enhanced Input (optional), GameplayAbilities (optional)

## 📞 Support

### Documentation Issues
If you find issues with this documentation:
- **GitHub Issues**: [Report Documentation Problems](https://github.com/DermanDanisman/GAS-RPG-TopDown/issues)
- **Suggest Improvements**: Submit pull requests for documentation updates

### Technical Support
For plugin technical support:
- **GitHub Issues**: [Technical Support](https://github.com/DermanDanisman/GAS-RPG-TopDown/issues)
- **Troubleshooting Guide**: [Common Issues and Solutions](Troubleshooting.md)

### Contributing
Contributions to improve the plugin and documentation are welcome:
- Follow existing code patterns and documentation style
- Include examples and test cases for new features
- Update relevant documentation sections

## 📄 License

© 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.

Unreal Engine and its associated trademarks are used under license from Epic Games.

---

**Ready to get started?** Begin with the [README](../README.md) for an overview, then follow the [Setup Guide](Setup-Guide.md) for step-by-step integration instructions.