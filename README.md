# Enigma Machine Simulator

A C++ implementation of the historical Enigma encryption machine.

# Developer

<p align="center">
  <!-- 👤 PROFILE PICTURE PLACEHOLDER -->
  <img src="cyiza.png" width="120" style="border-radius:50%" />
</p>

<p align="center">
  <b>Mpayimana Cyiza Landry</b><br/>
  Senior Software Engineer
</p>

---

## Features
- Complete Enigma Machine simulation
- Historical rotor wirings (Enigma I)
- Double-stepping mechanism
- Plugboard connections
- Configurable ring settings
- Modular arithmetic implementation
- Reversible encryption/decryption

## Components
- **Rotor**: 3 rotors with configurable positions, ring settings, and notches
- **Reflector**: Static wiring (B or C type)
- **Plugboard**: Configurable cable connections
- **EnigmaMachine**: Main class orchestrating the encryption process

## Installation
```bash
g++ -std=c++11 main.cpp -o enigma_simulator
./enigma_simulator