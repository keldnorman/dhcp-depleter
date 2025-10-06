# Contributing to DHCP Depleter

Thank you for your interest in contributing to DHCP Depleter! This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

By participating in this project, you agree to:

- Be respectful and inclusive
- Provide constructive feedback
- Focus on what is best for the community
- Use this tool only for legal, authorized testing
- Help others learn and grow

## How to Contribute

### Reporting Bugs

Before creating a bug report:
1. Check existing issues to avoid duplicates
2. Collect information about your setup
3. Try to reproduce the issue

When creating a bug report, include:
- ESP32-C6 board model and version
- PlatformIO version
- Operating system
- Complete serial monitor output
- Steps to reproduce
- Expected behavior
- Actual behavior
- Screenshots (if applicable)

**Bug Report Template:**
```markdown
**Description**
Clear description of the bug

**Environment**
- Board: ESP32-C6-DevKitC-1
- PlatformIO: 6.1.x
- OS: Windows/Mac/Linux

**Steps to Reproduce**
1. Step one
2. Step two
3. Step three

**Expected Behavior**
What should happen

**Actual Behavior**
What actually happens

**Serial Output**
```
Paste serial output here
```

**Screenshots**
If applicable
```

### Suggesting Enhancements

Enhancement suggestions are welcome! Please include:
- Clear description of the feature
- Use cases and benefits
- Potential implementation approach
- Examples from other projects (if applicable)

**Feature Request Template:**
```markdown
**Feature Description**
Clear description of the proposed feature

**Use Case**
Why is this feature needed?

**Proposed Solution**
How might this be implemented?

**Alternatives Considered**
Other approaches you've thought about

**Additional Context**
Any other relevant information
```

### Pull Requests

#### Before Starting

1. **Check existing PRs** to avoid duplicate work
2. **Open an issue** to discuss major changes
3. **Fork the repository** and create a branch
4. **Follow coding standards** (see below)

#### Development Process

1. **Fork the repository**
   ```bash
   # Fork on GitHub, then:
   git clone https://github.com/YOUR_USERNAME/dhcp-depleter.git
   cd dhcp-depleter
   ```

2. **Create a branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/bug-description
   ```

3. **Make your changes**
   - Write clean, documented code
   - Follow existing code style
   - Test thoroughly
   - Update documentation

4. **Test your changes**
   ```bash
   pio run
   pio run --target upload
   # Test on actual hardware
   ```

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add feature: description"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create Pull Request**
   - Go to the original repository
   - Click "New Pull Request"
   - Select your branch
   - Fill in the PR template

#### Pull Request Guidelines

- **Title**: Clear, concise description
- **Description**: Explain what and why
- **Link issues**: Reference related issues
- **Screenshots**: Include for UI changes
- **Testing**: Describe how you tested
- **Documentation**: Update relevant docs

**PR Template:**
```markdown
## Description
Clear description of changes

## Motivation and Context
Why is this change needed?

## Related Issues
Fixes #123

## Type of Change
- [ ] Bug fix (non-breaking change)
- [ ] New feature (non-breaking change)
- [ ] Breaking change (fix/feature that would break existing functionality)
- [ ] Documentation update

## How Has This Been Tested?
Describe testing process

## Screenshots (if applicable)

## Checklist
- [ ] Code follows project style
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings generated
- [ ] Tested on actual hardware
```

## Coding Standards

### C++ Style

#### Formatting
- **Indentation**: 2 spaces (no tabs)
- **Line length**: 100 characters maximum
- **Braces**: K&R style
```cpp
void function() {
  if (condition) {
    // code
  }
}
```

#### Naming Conventions
- **Functions**: camelCase
  ```cpp
  void handleRequest() { }
  ```
- **Variables**: snake_case
  ```cpp
  int request_count = 0;
  ```
- **Constants**: UPPER_CASE
  ```cpp
  const int MAX_RETRIES = 3;
  ```
- **Classes**: PascalCase
  ```cpp
  class NetworkManager { }
  ```

#### Comments
- Use clear, concise comments
- Explain "why" not "what"
- Document complex algorithms
```cpp
// Good
// Randomize MAC to appear as different device
randomizeMAC();

// Not needed
// Set variable to zero
count = 0;
```

#### Best Practices
- Keep functions small and focused
- Avoid global variables when possible
- Use const for read-only variables
- Check return values
- Handle errors gracefully
```cpp
if (WiFi.status() != WL_CONNECTED) {
  Serial.println("Connection failed");
  return false;
}
```

### Documentation Style

#### Markdown
- Use headers hierarchically (# ## ###)
- Use code blocks for commands/code
- Use lists for steps/items
- Include examples where helpful

#### Comments in Code
- Add file headers for new files
- Document public functions
- Explain complex logic
- Note any assumptions

## Testing

### Manual Testing Requirements

Before submitting a PR, test:
1. **Build**: Code compiles without errors/warnings
2. **Upload**: Firmware uploads successfully
3. **Boot**: Device boots and initializes
4. **AP Mode**: "Sinkhole" WiFi is created
5. **Web Interface**: UI loads correctly
6. **Scan**: Network scanning works
7. **Test**: Connection testing works
8. **Attack**: DHCP attack functions
9. **Stop**: Attack can be stopped
10. **Serial**: Output is clean and informative

### Test Checklist
- [ ] Fresh build completes successfully
- [ ] Device boots without errors
- [ ] Web interface loads on first try
- [ ] All buttons work correctly
- [ ] API endpoints return valid JSON
- [ ] Serial output is clean
- [ ] Attack successfully obtains IPs
- [ ] Attack can be stopped cleanly
- [ ] No memory leaks (observe over time)
- [ ] Works on target hardware

## Documentation

### When to Update Documentation

Update docs when you:
- Add new features
- Change existing behavior
- Fix bugs that affect usage
- Add configuration options
- Change API endpoints

### Which Files to Update

- **README.md**: Overview, features, usage
- **QUICKSTART.md**: Quick start instructions
- **BUILDING.md**: Build process changes
- **EXAMPLES.md**: New usage examples
- **SECURITY.md**: Security implications
- **ARCHITECTURE.md**: System design changes
- **CHANGELOG.md**: All changes

## Project Structure

```
dhcp-depleter/
├── src/
│   └── main.cpp          # Main application code
├── platformio.ini        # PlatformIO configuration
├── README.md             # Main documentation
├── QUICKSTART.md         # Quick start guide
├── BUILDING.md           # Build instructions
├── EXAMPLES.md           # Usage examples
├── SECURITY.md           # Legal/ethical guidelines
├── ARCHITECTURE.md       # System architecture
├── CONTRIBUTING.md       # This file
├── CHANGELOG.md          # Version history
├── LICENSE               # MIT license
└── .gitignore            # Git ignore rules
```

## Adding New Features

### Feature Development Process

1. **Discuss**: Open an issue to discuss the feature
2. **Design**: Plan the implementation
3. **Implement**: Write the code
4. **Test**: Verify functionality
5. **Document**: Update relevant documentation
6. **Submit**: Create pull request

### Example: Adding OTA Updates

1. **Issue**: "Add OTA firmware update support"
2. **Design**: Choose AsyncElegantOTA library
3. **Implementation**:
   - Add library to platformio.ini
   - Include headers in main.cpp
   - Add OTA initialization
   - Add web endpoint
4. **Testing**: Verify OTA works
5. **Documentation**: Update BUILDING.md, README.md
6. **PR**: Submit with description and screenshots

## Communication

### Channels
- **GitHub Issues**: Bug reports, feature requests
- **Pull Requests**: Code contributions
- **Discussions**: General questions, ideas

### Response Times
- We aim to respond to issues within 48 hours
- PR reviews may take up to 1 week
- Be patient and respectful

## Legal Considerations

### Responsible Disclosure

If you discover a security issue:
1. **Do not** create a public issue
2. Contact maintainers privately
3. Allow time for fix before disclosure
4. Follow responsible disclosure practices

### Code License

- All contributions will be under MIT license
- You retain copyright to your contributions
- You grant rights to use, modify, and distribute

### Legal Compliance

- Only contribute legal, ethical code
- Do not add features that encourage misuse
- Ensure contributions don't violate laws
- Consider international implications

## Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md (if created)
- Mentioned in release notes
- Credited in commit history

## Questions?

If you have questions about contributing:
1. Check existing documentation
2. Search closed issues
3. Open a new discussion
4. Contact maintainers

## Thank You!

Your contributions make this project better for everyone. Whether you're fixing bugs, adding features, or improving documentation, your help is appreciated!

---

**Remember**: This tool is for authorized security testing only. All contributions should support responsible, legal use of the software.
