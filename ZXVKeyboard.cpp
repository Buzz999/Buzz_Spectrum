
uint8_t keys[4][10] = {
  { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
  { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p' },
  { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n' },
  { ZX_CAPS_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ZX_SYMBOL_SHIFT, ' '}
};

sf::Keyboard::Key sfml_keys [4][10] = {
  { sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4, sf::Keyboard::Num5, sf::Keyboard::Num6, sf::Keyboard::Num7, sf::Keyboard::Num8, sf::Keyboard::Num9, sf::Keyboard::Num0 },
  { sf::Keyboard::Q, sf::Keyboard::W, sf::Keyboard::E, sf::Keyboard::R, sf::Keyboard::T, sf::Keyboard::Y, sf::Keyboard::U, sf::Keyboard::I, sf::Keyboard::O, sf::Keyboard::P},
  { sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D, sf::Keyboard::F, sf::Keyboard::G, sf::Keyboard::H, sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::L, sf::Keyboard::Return},
  { sf::Keyboard::LShift, sf::Keyboard::Z, sf::Keyboard::X, sf::Keyboard::C, sf::Keyboard::V, sf::Keyboard::B, sf::Keyboard::N, sf::Keyboard::M, sf::Keyboard::RShift}
};

void ZXVKeyboard::getRKeys (std::vector<uint8_t> &pressedkeys) const {
    for (uint8_t j=0; j<4; ++j) {
        for (uint8_t i=0; i < 10; ++i) {
            if (sf::Keyboard::isKeyPressed(sfml_keys[j][i])) {
                uint8_t k = keys[j][i];
                printf ("Key:%d\n", k);
                pressedkeys.push_back (k);
            }
        }
    }
}
