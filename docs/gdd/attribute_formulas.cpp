// attribute_formulas.cpp
// Simple reference implementations of the secondary-attribute formulas from the GDD.
// These functions are intended for use as documentation and quick testing. They are
// standalone and header-only compatible. Replace with game-specific implementations
// when integrating with your AttributeSet/GameplayEffect pipelines.

#include <algorithm>
#include <cmath>

// Utility function to clamp values within a range
float Clamp(float value, float min, float max)
{
    return std::max(min, std::min(value, max));
}

// Primary Attributes Input Structure (for reference)
struct PrimaryAttributes
{
    float Strength = 0.0f;
    float Dexterity = 0.0f;
    float Intelligence = 0.0f;
    float Endurance = 0.0f;
    float Vigor = 0.0f;
};

// Additional Input Parameters
struct CombatParameters
{
    float WeaponDamage = 0.0f;
    float SpellBase = 0.0f;
};

// Secondary Attribute Calculations
// All formulas based on GDD Section 4: "Secondary stats & sample formulas"

/**
 * Calculate physical attack power
 * Formula: Attack Power (physical) = (1.5 × (Strength + WeaponDamage)) + 0
 */
float CalculateAttackPower(const PrimaryAttributes& Attrs, float WeaponDamage)
{
    return 1.5f * (Attrs.Strength + WeaponDamage);
}

/**
 * Calculate spell power for magical abilities
 * Formula: Spell Power = (1.5 × (Intelligence + SpellBase)) + 0
 */
float CalculateSpellPower(const PrimaryAttributes& Attrs, float SpellBase)
{
    return 1.5f * (Attrs.Intelligence + SpellBase);
}

/**
 * Calculate base armor value
 * Formula: Armor = 1.25 * (Endurance + 5)
 */
float CalculateArmor(const PrimaryAttributes& Attrs)
{
    return 1.25f * (Attrs.Endurance + 5.0f);
}

/**
 * Calculate armor penetration percentage
 * Formula: Armor Penetration (%) = 0.45 * (Strength + 3)
 */
float CalculateArmorPenetration(const PrimaryAttributes& Attrs)
{
    return 0.45f * (Attrs.Strength + 3.0f);
}

/**
 * Calculate block chance percentage (clamped to 0-60%)
 * Formula: Block Chance (%) = clamp( (Armor * 0.2), 0, 60 )
 */
float CalculateBlockChance(float Armor)
{
    return Clamp(Armor * 0.2f, 0.0f, 60.0f);
}

/**
 * Calculate critical hit chance percentage (clamped to 0-95%)
 * Formula: Crit Chance (%) = clamp( 0.4 * (Dexterity + 2) + (ArmorPenetration * 0.1), 0, 95 )
 */
float CalculateCritChance(const PrimaryAttributes& Attrs, float ArmorPenetration)
{
    float CritChance = 0.4f * (Attrs.Dexterity + 2.0f) + (ArmorPenetration * 0.1f);
    return Clamp(CritChance, 0.0f, 95.0f);
}

/**
 * Calculate critical hit damage percentage
 * Formula: Crit Damage (%) = (1.15 * Dexterity) + (ArmorPenetration * 0.2) + 50
 */
float CalculateCritDamage(const PrimaryAttributes& Attrs, float ArmorPenetration)
{
    return (1.15f * Attrs.Dexterity) + (ArmorPenetration * 0.2f) + 50.0f;
}

/**
 * Calculate critical hit resistance percentage
 * Formula: Crit Resistance (%) = (0.5 * Armor)
 */
float CalculateCritResistance(float Armor)
{
    return 0.5f * Armor;
}

/**
 * Calculate evasion chance percentage
 * Formula: Evasion (%) = (0.3 * (Dexterity + Endurance)) + 2
 */
float CalculateEvasion(const PrimaryAttributes& Attrs)
{
    return (0.3f * (Attrs.Dexterity + Attrs.Endurance)) + 2.0f;
}

/**
 * Calculate maximum health points
 * Formula: Max Health = (10 * Vigor) + 50
 */
float CalculateMaxHealth(const PrimaryAttributes& Attrs)
{
    return (10.0f * Attrs.Vigor) + 50.0f;
}

/**
 * Calculate health regeneration rate (HP per second)
 * Formula: Health Regen (HP/sec) = (0.5 * (Vigor + 1))
 */
float CalculateHealthRegen(const PrimaryAttributes& Attrs)
{
    return 0.5f * (Attrs.Vigor + 1.0f);
}

/**
 * Calculate maximum mana points
 * Formula: Max Mana = (5 * Intelligence) + 25
 */
float CalculateMaxMana(const PrimaryAttributes& Attrs)
{
    return (5.0f * Attrs.Intelligence) + 25.0f;
}

/**
 * Calculate mana regeneration rate (MP per second)
 * Formula: Mana Regen (MP/sec) = 1.0 * (Intelligence) + 3
 */
float CalculateManaRegen(const PrimaryAttributes& Attrs)
{
    return (1.0f * Attrs.Intelligence) + 3.0f;
}

/**
 * Calculate maximum stamina points
 * Formula: Max Stamina = (10 * Vigor) + 50
 */
float CalculateMaxStamina(const PrimaryAttributes& Attrs)
{
    return (10.0f * Attrs.Vigor) + 50.0f;
}

/**
 * Calculate stamina regeneration rate (stamina per second)
 * Formula: Stamina Regen = (0.5 * (Vigor + 1))
 */
float CalculateStaminaRegen(const PrimaryAttributes& Attrs)
{
    return 0.5f * (Attrs.Vigor + 1.0f);
}

// Complete secondary attribute calculation function
// This demonstrates how all formulas work together
struct SecondaryAttributes
{
    float AttackPower = 0.0f;
    float SpellPower = 0.0f;
    float Armor = 0.0f;
    float ArmorPenetration = 0.0f;
    float BlockChance = 0.0f;
    float CritChance = 0.0f;
    float CritDamage = 0.0f;
    float CritResistance = 0.0f;
    float Evasion = 0.0f;
    float MaxHealth = 0.0f;
    float HealthRegen = 0.0f;
    float MaxMana = 0.0f;
    float ManaRegen = 0.0f;
    float MaxStamina = 0.0f;
    float StaminaRegen = 0.0f;
};

/**
 * Calculate all secondary attributes from primary attributes and combat parameters
 * This function demonstrates the complete attribute derivation pipeline
 */
SecondaryAttributes CalculateAllSecondaryAttributes(const PrimaryAttributes& Primary, const CombatParameters& Combat)
{
    SecondaryAttributes Secondary;

    // Calculate derived stats that don't depend on other derived stats
    Secondary.AttackPower = CalculateAttackPower(Primary, Combat.WeaponDamage);
    Secondary.SpellPower = CalculateSpellPower(Primary, Combat.SpellBase);
    Secondary.Armor = CalculateArmor(Primary);
    Secondary.ArmorPenetration = CalculateArmorPenetration(Primary);
    Secondary.Evasion = CalculateEvasion(Primary);
    Secondary.MaxHealth = CalculateMaxHealth(Primary);
    Secondary.HealthRegen = CalculateHealthRegen(Primary);
    Secondary.MaxMana = CalculateMaxMana(Primary);
    Secondary.ManaRegen = CalculateManaRegen(Primary);
    Secondary.MaxStamina = CalculateMaxStamina(Primary);
    Secondary.StaminaRegen = CalculateStaminaRegen(Primary);

    // Calculate derived stats that depend on other derived stats
    Secondary.BlockChance = CalculateBlockChance(Secondary.Armor);
    Secondary.CritChance = CalculateCritChance(Primary, Secondary.ArmorPenetration);
    Secondary.CritDamage = CalculateCritDamage(Primary, Secondary.ArmorPenetration);
    Secondary.CritResistance = CalculateCritResistance(Secondary.Armor);

    return Secondary;
}

// Example usage and testing functions

/**
 * Example function demonstrating how to use the attribute formulas
 * with sample primary attribute values from the GDD level tables
 */
void ExampleUsage()
{
    // Example: Level 10 Warrior from GDD progression table
    PrimaryAttributes WarriorL10;
    WarriorL10.Strength = 26.0f;
    WarriorL10.Dexterity = 12.0f;
    WarriorL10.Intelligence = 7.0f;
    WarriorL10.Endurance = 16.0f;
    WarriorL10.Vigor = 19.0f;

    CombatParameters CombatParams;
    CombatParams.WeaponDamage = 15.0f;  // Example weapon damage
    CombatParams.SpellBase = 5.0f;      // Example spell base

    // Calculate all secondary attributes
    SecondaryAttributes Results = CalculateAllSecondaryAttributes(WarriorL10, CombatParams);

    // Results can be used for gameplay calculations, UI display, or validation testing
    // Example values for Level 10 Warrior:
    // - Attack Power: 61.5 (1.5 * (26 + 15))
    // - Max Health: 240 (10 * 19 + 50)
    // - Armor: 26.25 (1.25 * (16 + 5))
    // - Block Chance: 5.25% (26.25 * 0.2, clamped to 0-60%)
}

// Test functions for validation (can be removed in production)
#ifdef TESTING_ENABLED

bool TestAttributeFormulas()
{
    // Simple test with known values
    PrimaryAttributes TestAttrs;
    TestAttrs.Strength = 10.0f;
    TestAttrs.Dexterity = 10.0f;
    TestAttrs.Intelligence = 10.0f;
    TestAttrs.Endurance = 10.0f;
    TestAttrs.Vigor = 10.0f;

    // Test individual formulas with expected results
    float Armor = CalculateArmor(TestAttrs);
    if (std::abs(Armor - 18.75f) > 0.01f) return false; // 1.25 * (10 + 5) = 18.75

    float MaxHealth = CalculateMaxHealth(TestAttrs);
    if (std::abs(MaxHealth - 150.0f) > 0.01f) return false; // 10 * 10 + 50 = 150

    float ArmorPen = CalculateArmorPenetration(TestAttrs);
    if (std::abs(ArmorPen - 5.85f) > 0.01f) return false; // 0.45 * (10 + 3) = 5.85

    return true;
}

#endif // TESTING_ENABLED