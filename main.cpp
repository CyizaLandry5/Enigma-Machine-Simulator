#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <stdexcept>

// Constants
const int ALPHABET_SIZE = 26;
const char FIRST_LETTER = 'A';

/**
 * Utility function to convert character to index (0-25)
 */
int charToIndex(char c) {
    return std::toupper(c) - FIRST_LETTER;
}

/**
 * Utility function to convert index to character
 */
char indexToChar(int index) {
    return static_cast<char>(FIRST_LETTER + (index % ALPHABET_SIZE));
}

/**
 * Base class for all Enigma components
 */
class EnigmaComponent {
protected:
    std::string wiring;
    std::string name;
    int position;
    int ringSetting;
    
public:
    EnigmaComponent(const std::string& wiring, const std::string& name = "Component")
        : wiring(wiring), name(name), position(0), ringSetting(0) {
        if (wiring.length() != ALPHABET_SIZE) {
            throw std::invalid_argument("Wiring must be exactly 26 characters");
        }
    }
    
    virtual ~EnigmaComponent() = default;
    
    virtual char process(char input, bool forward = true) = 0;
    
    void setPosition(int pos) {
        position = pos % ALPHABET_SIZE;
    }
    
    void setRingSetting(int setting) {
        ringSetting = setting % ALPHABET_SIZE;
    }
    
    int getPosition() const {
        return position;
    }
    
    std::string getName() const {
        return name;
    }
    
    void rotate() {
        position = (position + 1) % ALPHABET_SIZE;
    }
    
    /**
     * Apply position and ring setting to a signal
     */
    int applyOffset(int signal, bool forward) const {
        int offset = position - ringSetting;
        if (offset < 0) offset += ALPHABET_SIZE;
        
        if (forward) {
            signal = (signal + offset) % ALPHABET_SIZE;
        } else {
            signal = (signal - offset + ALPHABET_SIZE) % ALPHABET_SIZE;
        }
        
        return signal;
    }
};

/**
 * Rotor class - represents a single Enigma rotor
 */
class Rotor : public EnigmaComponent {
private:
    std::string reverseWiring;
    int notchPosition;
    
    void buildReverseWiring() {
        reverseWiring.resize(ALPHABET_SIZE);
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            char output = wiring[i];
            int outputIndex = charToIndex(output);
            reverseWiring[outputIndex] = indexToChar(i);
        }
    }
    
public:
    Rotor(const std::string& wiring, int notch, const std::string& name = "Rotor")
        : EnigmaComponent(wiring, name), notchPosition(notch) {
        buildReverseWiring();
    }
    
    char process(char input, bool forward = true) override {
        int signal = charToIndex(input);
        
        // Apply position and ring setting
        signal = applyOffset(signal, forward);
        
        // Apply wiring transformation
        if (forward) {
            char outputChar = wiring[signal];
            signal = charToIndex(outputChar);
        } else {
            char outputChar = reverseWiring[signal];
            signal = charToIndex(outputChar);
        }
        
        // Reverse position and ring setting
        signal = applyOffset(signal, !forward);
        
        return indexToChar(signal);
    }
    
    bool isAtNotch() const {
        return position == notchPosition;
    }
    
    void setNotch(int notch) {
        notchPosition = notch % ALPHABET_SIZE;
    }
};

/**
 * Reflector class - reflects signals back through rotors
 */
class Reflector : public EnigmaComponent {
public:
    Reflector(const std::string& wiring, const std::string& name = "Reflector")
        : EnigmaComponent(wiring, name) {}
    
    char process(char input, bool forward = true) override {
        // Reflector only works in one direction
        int signal = charToIndex(input);
        char outputChar = wiring[signal];
        return outputChar;
    }
};

/**
 * Plugboard class - implements the cable connections
 */
class Plugboard {
private:
    std::map<char, char> connections;
    
public:
    Plugboard() = default;
    
    void connect(char a, char b) {
        a = std::toupper(a);
        b = std::toupper(b);
        
        // Check if letters are already connected
        if (connections.find(a) != connections.end() || connections.find(b) != connections.end()) {
            throw std::invalid_argument("One or both letters are already connected");
        }
        
        connections[a] = b;
        connections[b] = a;
    }
    
    void clearConnections() {
        connections.clear();
    }
    
    char process(char input) {
        input = std::toupper(input);
        
        // If connected, return the paired letter
        auto it = connections.find(input);
        if (it != connections.end()) {
            return it->second;
        }
        
        // Otherwise, return the input unchanged
        return input;
    }
    
    std::string getConnections() const {
        std::string result;
        std::vector<bool> added(ALPHABET_SIZE, false);
        
        for (const auto& pair : connections) {
            char a = pair.first;
            char b = pair.second;
            
            if (!added[charToIndex(a)]) {
                result += a;
                result += b;
                result += " ";
                added[charToIndex(a)] = true;
                added[charToIndex(b)] = true;
            }
        }
        
        return result;
    }
};

/**
 * Main Enigma Machine class
 */
class EnigmaMachine {
private:
    std::vector<Rotor> rotors;
    Reflector reflector;
    Plugboard plugboard;
    
    // Rotate the rotors according to Enigma stepping mechanism
    void rotateRotors() {
        // Always rotate the rightmost rotor
        rotors[2].rotate();
        
        // Check for double-stepping and notch positions
        bool rotateMiddle = rotors[2].isAtNotch();
        bool rotateLeft = rotors[1].isAtNotch();
        
        // Double-stepping mechanism
        if (rotateMiddle) {
            rotors[1].rotate();
            rotors[0].rotate(); // Left rotor also rotates when middle rotor is at notch
        }
        
        // Normal notch-triggered rotation
        if (rotateLeft) {
            rotors[0].rotate();
        }
    }
    
public:
    EnigmaMachine(const std::vector<Rotor>& rotors, const Reflector& reflector)
        : rotors(rotors), reflector(reflector) {
        if (rotors.size() != 3) {
            throw std::invalid_argument("Enigma machine requires exactly 3 rotors");
        }
    }
    
    char encryptChar(char input) {
        // Step 1: Rotate rotors before encryption
        rotateRotors();
        
        // Step 2: Plugboard transformation
        char result = plugboard.process(input);
        
        // Step 3: Forward pass through rotors (right to left)
        for (int i = rotors.size() - 1; i >= 0; i--) {
            result = rotors[i].process(result, true);
        }
        
        // Step 4: Reflector
        result = reflector.process(result);
        
        // Step 5: Backward pass through rotors (left to right)
        for (size_t i = 0; i < rotors.size(); i++) {
            result = rotors[i].process(result, false);
        }
        
        // Step 6: Plugboard again
        result = plugboard.process(result);
        
        return result;
    }
    
    std::string encrypt(const std::string& message) {
        std::string result;
        
        for (char c : message) {
            if (std::isalpha(c)) {
                result += encryptChar(c);
            } else {
                // Non-alphabetic characters are passed through unchanged
                result += c;
            }
        }
        
        return result;
    }
    
    void setRotorPositions(int left, int middle, int right) {
        rotors[0].setPosition(left);
        rotors[1].setPosition(middle);
        rotors[2].setPosition(right);
    }
    
    void setRingSettings(int left, int middle, int right) {
        rotors[0].setRingSetting(left);
        rotors[1].setRingSetting(middle);
        rotors[2].setRingSetting(right);
    }
    
    void setPlugboardConnections(const std::vector<std::pair<char, char>>& connections) {
        plugboard.clearConnections();
        for (const auto& connection : connections) {
            plugboard.connect(connection.first, connection.second);
        }
    }
    
    std::string getCurrentState() const {
        std::string state;
        state += "Rotor Positions: ";
        state += indexToChar(rotors[0].getPosition());
        state += indexToChar(rotors[1].getPosition());
        state += indexToChar(rotors[2].getPosition());
        state += "\nPlugboard: " + plugboard.getConnections();
        return state;
    }
    
    Plugboard& getPlugboard() {
        return plugboard;
    }
};

/**
 * Factory functions to create historical Enigma components
 */
namespace EnigmaFactory {
    // Historical rotor wirings (Commercial Enigma I)
    Rotor createRotorI(int position = 0, int ringSetting = 0) {
        Rotor rotor("EKMFLGDQVZNTOWYHXUSPAIBRCJ", 16, "Rotor I");
        rotor.setPosition(position);
        rotor.setRingSetting(ringSetting);
        return rotor;
    }
    
    Rotor createRotorII(int position = 0, int ringSetting = 0) {
        Rotor rotor("AJDKSIRUXBLHWTMCQGZNPYFVOE", 4, "Rotor II");
        rotor.setPosition(position);
        rotor.setRingSetting(ringSetting);
        return rotor;
    }
    
    Rotor createRotorIII(int position = 0, int ringSetting = 0) {
        Rotor rotor("BDFHJLCPRTXVZNYEIWGAKMUSQO", 21, "Rotor III");
        rotor.setPosition(position);
        rotor.setRingSetting(ringSetting);
        return rotor;
    }
    
    // Reflector B (most common)
    Reflector createReflectorB() {
        return Reflector("YRUHQSLDPXNGOKMIEBFZCWVJAT", "Reflector B");
    }
    
    // Reflector C
    Reflector createReflectorC() {
        return Reflector("FVPJIAOYEDRZXWGCTKUQSBNMHL", "Reflector C");
    }
}

/**
 * Main program with example usage
 */
int main() {
    std::cout << "=========================================\n";
    std::cout << "      ENIGMA MACHINE SIMULATOR\n";
    std::cout << "=========================================\n\n";
    
    try {
        // Create Enigma machine with historical settings
        std::vector<Rotor> rotors = {
            EnigmaFactory::createRotorI(0, 0),    // Left rotor
            EnigmaFactory::createRotorII(0, 0),   // Middle rotor
            EnigmaFactory::createRotorIII(0, 0)   // Right rotor
        };
        
        Reflector reflector = EnigmaFactory::createReflectorB();
        EnigmaMachine enigma(rotors, reflector);
        
        // Set initial rotor positions (example: A, B, C)
        enigma.setRotorPositions(0, 1, 2);  // A=0, B=1, C=2
        
        // Set ring settings (example: all at A)
        enigma.setRingSettings(0, 0, 0);
        
        // Configure plugboard (example connections)
        std::vector<std::pair<char, char>> plugboardConnections = {
            {'A', 'B'},
            {'C', 'D'},
            {'E', 'F'}
        };
        enigma.setPlugboardConnections(plugboardConnections);
        
        // Display initial configuration
        std::cout << "Initial Configuration:\n";
        std::cout << enigma.getCurrentState() << "\n\n";
        
        // Example 1: Encrypt a single character
        std::cout << "Example 1: Encrypting 'H' -> ";
        char encrypted = enigma.encryptChar('H');
        std::cout << encrypted << "\n";
        
        // Example 2: Encrypt a message
        std::string message = "HELLOENIGMA";
        std::cout << "\nExample 2: Encrypting message\n";
        std::cout << "Original:  " << message << "\n";
        
        // Reset to same starting position for encryption
        enigma.setRotorPositions(0, 1, 2);
        std::string encryptedMessage = enigma.encrypt(message);
        std::cout << "Encrypted: " << encryptedMessage << "\n";
        
        // Example 3: Decrypt the message (Enigma is symmetric)
        std::cout << "\nExample 3: Decrypting message\n";
        enigma.setRotorPositions(0, 1, 2);  // Reset to same starting position
        std::string decryptedMessage = enigma.encrypt(encryptedMessage);
        std::cout << "Decrypted: " << decryptedMessage << "\n";
        
        // Example 4: Interactive encryption
        std::cout << "\n=========================================\n";
        std::cout << "INTERACTIVE ENCRYPTION DEMO\n";
        std::cout << "=========================================\n";
        
        // Create a new machine with different settings
        std::vector<Rotor> rotors2 = {
            EnigmaFactory::createRotorIII(5, 1),
            EnigmaFactory::createRotorII(10, 2),
            EnigmaFactory::createRotorI(15, 3)
        };
        
        EnigmaMachine enigma2(rotors2, EnigmaFactory::createReflectorB());
        
        // Set plugboard connections
        std::vector<std::pair<char, char>> connections2 = {
            {'Q', 'W'},
            {'E', 'R'},
            {'T', 'Y'},
            {'U', 'I'},
            {'O', 'P'}
        };
        enigma2.setPlugboardConnections(connections2);
        
        std::cout << "\nMachine configured with:\n";
        std::cout << "Rotor Order: III, II, I (from left to right)\n";
        std::cout << "Rotor Positions: F, K, P (5, 10, 15)\n";
        std::cout << "Ring Settings: B, C, D (1, 2, 3)\n";
        std::cout << "Plugboard: " << enigma2.getPlugboard().getConnections() << "\n\n";
        
        // Encrypt a sample message
        std::string sample = "THEQUICKBROWNFOXJUMPSOVERTHELAZYDOG";
        std::cout << "Original message:\n" << sample << "\n\n";
        
        std::string encryptedSample = enigma2.encrypt(sample);
        std::cout << "Encrypted message:\n" << encryptedSample << "\n\n";
        
        // Reset and decrypt to verify
        enigma2.setRotorPositions(5, 10, 15);
        std::string decryptedSample = enigma2.encrypt(encryptedSample);
        std::cout << "Decrypted message:\n" << decryptedSample << "\n";
        
        // Demonstrate the stepping mechanism
        std::cout << "\n=========================================\n";
        std::cout << "ROTOR STEPPING DEMONSTRATION\n";
        std::cout << "=========================================\n";
        
        // Reset to a position near a notch
        std::vector<Rotor> demoRotors = {
            EnigmaFactory::createRotorI(0, 0),
            EnigmaFactory::createRotorII(0, 0),
            EnigmaFactory::createRotorIII(20, 0)  // Near notch at 21 (V)
        };
        
        EnigmaMachine demoEnigma(demoRotors, EnigmaFactory::createReflectorB());
        demoEnigma.setRotorPositions(0, 0, 20);  // A, A, U
        
        std::cout << "\nInitial positions: A A U\n";
        
        for (int i = 0; i < 5; i++) {
            char test = demoEnigma.encryptChar('A');
            std::cout << "After encryption " << i + 1 << ": Positions: ";
            std::cout << indexToChar(demoRotors[0].getPosition()) << " ";
            std::cout << indexToChar(demoRotors[1].getPosition()) << " ";
            std::cout << indexToChar(demoRotors[2].getPosition());
            std::cout << " (Encrypted 'A' -> '" << test << "')\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=========================================\n";
    std::cout << "Enigma simulation completed successfully!\n";
    std::cout << "=========================================\n";
    
    return 0;
}
