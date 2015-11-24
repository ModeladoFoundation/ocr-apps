#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAXATOMS 64
#define MAXSPECIES 1

// The units for this code are:
//     - Time in femtoseconds (fs)
//     - Length in Angstroms (Angs)
//     - Energy in electron Volts (eV)
//     - Force in eV/Angstrom
//     - Mass read in as Atomic Mass Units (amu) and then converted for
//       consistency (energy*time^2/length^2)
// Values are taken from NIST, http://physics.nist.gov/cuu/Constants/

/// 1 amu in kilograms
#define amu2Kilograms  1.660538921e-27

/// 1 fs in seconds
#define fs2seconds     1.0e-15

/// 1 Ang in meters
#define Angs2meters    1.0e-10

/// 1 eV in Joules
#define eV2Joules      1.602176565e-19

/// Internal mass units are eV * fs^2 / Ang^2
static const double amu2internal_mass =
         amu2Kilograms * Angs2meters * Angs2meters
         / (fs2seconds * fs2seconds  * eV2Joules);

/// Boltmann constant in eV's
static const double kB_eV = 8.6173324e-5;  // eV/K

/// Conversion to Temperature
static const double kB_eV_1_5 = 8.6173324e-5*1.5;  // K/eV

/// Hartrees to eVs
static const double hartree2Ev = 27.21138505;

/// Bohrs to Angstroms
static const double bohr2Angs = 0.52917721092;

#define POT_SHIFT 1.0

#endif
