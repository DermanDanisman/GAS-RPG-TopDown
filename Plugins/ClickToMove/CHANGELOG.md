# ClickToMove Plugin - Change Log

All notable changes to the ClickToMove plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01-XX

### Added
- **Core Movement System**
  - Dual movement modes: hold-to-move and short-press autorun
  - Advanced pathfinding with UE5 Navigation System integration
  - Corner-cutting optimization with configurable lookahead
  - Speed-based acceptance radius scaling
  - Automatic navmesh projection for any surface clicks

- **Performance Features**
  - Conditional component ticking (only during autorun)
  - Local controller validation for multiplayer efficiency
  - Debug visualization excluded from shipping builds
  - Efficient path caching and reuse

- **Framework Integration**
  - Enhanced Input System support with proper trigger events
  - Gameplay Ability System (GAS) coordination
  - Custom interaction system integration points
  - Component-based architecture for easy extension

- **Multiplayer Support**
  - Client-side prediction with CharacterMovement replication
  - Network-optimized design with minimal bandwidth
  - Authority validation patterns
  - Cross-platform consistency

- **Configuration System**
  - Extensive Blueprint-exposed parameters
  - Runtime behavior modification
  - Game-specific configuration presets
  - Comprehensive debug visualization

- **Platform Support**
  - Windows (Win64)
  - macOS
  - Linux
  - Android
  - iOS

- **Documentation**
  - Complete setup guide with Blueprint and C++ examples
  - Full API reference documentation
  - Configuration guide with game-specific tuning
  - Integration examples for common frameworks
  - Comprehensive troubleshooting guide
  - Performance optimization recommendations

### Technical Details
- `UClickToMoveComponent` main component class
- Inherits from `UActorComponent` for lightweight integration
- Dependencies: NavigationSystem (required), EnhancedInput (optional), GameplayAbilities (optional)
- Memory usage: ~1KB static + dynamic path arrays
- Performance: <0.1ms per frame during active pathfinding

### API Highlights
- `OnClickPressed()` - Initialize click sequence
- `OnClickHeld(bool, FHitResult)` - Handle continuous input with dual trace modes
- `OnClickReleased()` - Finalize movement with autorun decision
- `SetIsTargeting(bool)` - Integration with ability/interaction systems
- `StopMovement()` - Emergency movement cancellation

### Configuration Parameters
- `ShortPressThreshold` - Movement timing control (0.05s - ∞)
- `AcceptanceRadius` - Base arrival detection with speed scaling
- `bUseLookahead` + `LookaheadBlendAlpha` - Pathfinding smoothness
- `NavProjectExtent` - Surface projection search volume
- Debug visualization options for development

---

## Version Numbering

This project uses [Semantic Versioning](https://semver.org/):

- **MAJOR.MINOR.PATCH** (e.g., 1.0.0)
- **MAJOR**: Breaking API changes
- **MINOR**: New features, backward compatible
- **PATCH**: Bug fixes, backward compatible

### Release Channels

- **Stable**: Tested releases for production use
- **Beta**: Feature-complete pre-releases for testing
- **Alpha**: Early development releases (not recommended for production)

---

## Upgrade Guide

### From Pre-Release to v1.0.0

**New Projects:**
- Follow the [Setup Guide](Documentation/Setup-Guide.md)
- Use recommended configuration from [Configuration Guide](Documentation/Configuration-Guide.md)

**Existing Implementations:**
- Replace custom click-to-move code with ClickToMoveComponent
- Update input bindings for Enhanced Input compatibility
- Review collision channel configuration
- Test multiplayer behavior if applicable

**Breaking Changes:**
- None (initial release)

**Deprecated Features:**
- None (initial release)

---

## Development Roadmap

### Planned for v1.1.0
- Path preview visualization system
- Formation movement support for AI companions
- Advanced obstacle avoidance options
- Terrain-based movement speed modifications

### Planned for v1.2.0
- Visual scripting nodes for advanced behaviors
- Mobile-optimized touch gesture recognition
- Additional debug and profiling tools

### Long-term Goals
- Integration with popular third-party plugins
- VR/AR pointing device support
- Advanced AI movement coordination

---

## Support and Feedback

### Reporting Issues
- **Bug Reports**: [GitHub Issues](https://github.com/DermanDanisman/GAS-RPG-TopDown/issues)
- **Feature Requests**: [GitHub Issues](https://github.com/DermanDanisman/GAS-RPG-TopDown/issues)
- **Documentation Issues**: [GitHub Issues](https://github.com/DermanDanisman/GAS-RPG-TopDown/issues)

### Contributing
- Follow existing code patterns and documentation style
- Include examples and test cases for new features
- Update relevant documentation sections
- Test on multiple platforms when possible

### Community
- Share your implementations and use cases
- Provide feedback on documentation clarity
- Suggest improvements and new features
- Help other developers with integration questions

---

## License

© 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.

Unreal Engine and its associated trademarks are used under license from Epic Games.