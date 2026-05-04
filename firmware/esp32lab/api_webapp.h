#pragma once

// Registers HTTP routes that serve the web app directly from firmware flash.
// No LittleFS or separate data upload needed.
void setupWebApp();
