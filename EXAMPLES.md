# Example Usage Scenarios

This document provides detailed examples of how to use the DHCP Depleter for various testing scenarios.

## Scenario 1: Home Network Testing

### Objective
Test the resilience of your home WiFi router's DHCP server.

### Setup
1. Power on the ESP32-C6 device
2. Connect your laptop/phone to "Sinkhole" WiFi (password: 12345678)
3. Open browser to http://10.13.37.1

### Steps
1. **Scan Networks**
   - Click "Scan WiFi Networks"
   - You should see your home network in the list

2. **Select Target**
   - Choose your home network from the dropdown
   - Enter your WiFi password

3. **Test Connection**
   - Click "Test Connection"
   - Wait for confirmation (should show your assigned IP)

4. **Start Attack**
   - Click "Start Attack"
   - Monitor the statistics in real-time
   - Observe when your DHCP pool becomes exhausted

5. **Verify Impact**
   - Try connecting another device to your network
   - It should fail to get an IP address

6. **Stop and Restore**
   - Click "Stop Attack"
   - Reboot your router to release DHCP leases
   - Verify normal operation

### Expected Results
- Small home routers typically have 50-254 available addresses
- Attack should exhaust pool in 1-5 minutes depending on lease time
- New devices will be unable to connect

---

## Scenario 2: Enterprise Network Assessment

### Objective
Assess enterprise WiFi infrastructure's resilience to DHCP attacks.

### Prerequisites
- Written authorization from network owner
- Scheduled maintenance window
- Backup connectivity plan
- Stakeholder notification

### Setup
1. Connect to enterprise test network (not production!)
2. Document baseline DHCP behavior
3. Set up monitoring on DHCP servers
4. Prepare incident response procedures

### Testing Process

#### Phase 1: Reconnaissance
```
1. Scan for available networks
2. Identify test SSID
3. Document DHCP server details
4. Note current address pool size
```

#### Phase 2: Baseline Testing
```
1. Connect with 1 device
2. Record lease time
3. Document pool capacity
4. Test normal DHCP behavior
```

#### Phase 3: Controlled Attack
```
1. Start attack with monitoring
2. Document addresses obtained
3. Monitor server response
4. Observe at what point pool exhausts
5. Check for rate limiting or blocking
```

#### Phase 4: Analysis
```
1. Stop attack immediately
2. Review DHCP server logs
3. Check security system alerts
4. Document findings
5. Calculate time to exhaust pool
```

### Monitoring Checklist
- [ ] DHCP server logs enabled
- [ ] Network monitoring active
- [ ] Security alerts configured
- [ ] Backup connectivity verified
- [ ] Stakeholders notified

### Report Template
```
DHCP Resilience Test Report

Network: [Enterprise Test Network]
Date: [Test Date]
Authorization: [Reference Number]

Test Results:
- Pool Size: [Number] addresses
- Lease Time: [Duration]
- Time to Exhaust: [Duration]
- Addresses Obtained: [Number]
- Rate Limiting: [Yes/No]
- Security Alerts: [Triggered/Not Triggered]

Vulnerabilities Found:
1. [Description]
2. [Description]

Recommendations:
1. [Recommendation]
2. [Recommendation]

Tester: [Name]
Signature: [Signature]
```

---

## Scenario 3: Security Training Lab

### Objective
Demonstrate DHCP vulnerabilities to security students.

### Lab Setup
1. Create isolated network with WiFi access point
2. Configure DHCP server with small pool (e.g., 10 addresses)
3. Set short lease time (e.g., 5 minutes)
4. Enable logging on all network devices

### Student Exercise

#### Part 1: Understanding DHCP
Students learn:
- How DHCP works
- DHCP message types (DISCOVER, OFFER, REQUEST, ACK)
- Lease time concepts
- Address pool management

#### Part 2: Observing Normal Behavior
Students:
1. Connect devices normally
2. Capture DHCP traffic with Wireshark
3. Analyze DHCP packets
4. Document lease process

#### Part 3: Attack Demonstration
Instructor:
1. Powers on DHCP Depleter
2. Demonstrates web interface
3. Starts attack against lab network
4. Shows real-time statistics

Students observe:
- MAC address randomization
- Rapid DHCP requests
- Pool exhaustion
- Service denial to legitimate clients

#### Part 4: Defense Implementation
Students configure:
- DHCP snooping
- Port security
- Rate limiting
- Monitoring and alerting

#### Part 5: Validation
Students verify:
- Attack is now mitigated
- Legitimate traffic still works
- Alerts are triggered
- Logs capture attack attempts

### Learning Outcomes
- Understanding of DHCP protocol
- Recognition of DoS attack patterns
- Implementation of network security controls
- Analysis of security logs

---

## Scenario 4: IoT Device Network Testing

### Objective
Test DHCP resilience in IoT environment with many devices.

### Context
IoT networks often have:
- Many devices (100-1000+)
- Devices that reconnect frequently
- Limited DHCP pool sizes
- Critical uptime requirements

### Test Configuration
```
Network: IoT Test VLAN
DHCP Pool: 10.20.30.100 - 10.20.30.200 (100 addresses)
Lease Time: 1 hour
Critical Devices: 20
Non-critical Devices: 50
```

### Test Procedure

1. **Baseline Measurement**
   ```
   Current usage: 70/100 addresses
   Peak usage: 85/100 addresses
   Typical churn rate: 10 devices/hour
   ```

2. **Gradual Attack**
   - Start attack
   - Monitor at what point critical services fail
   - Document behavior

3. **Analysis**
   ```
   - Time to exhaust remaining pool: [X] minutes
   - First service failure: [Service name] at [Y] addresses
   - Critical impact threshold: [Z] addresses remaining
   ```

4. **Recommendations**
   ```
   - Increase pool size to [N] addresses
   - Implement DHCP reservation for critical devices
   - Configure shorter lease time of [T] minutes
   - Enable rate limiting at [R] requests/minute
   ```

---

## Scenario 5: Guest Network Testing

### Objective
Verify guest network isolation and DHCP protection.

### Setup
- Guest network with captive portal
- Isolated from corporate network
- Limited DHCP pool

### Test Goals
1. Verify guest network can be exhausted
2. Confirm corporate network is unaffected
3. Test rate limiting effectiveness
4. Validate monitoring and alerts

### Expected Behavior
```
Guest Network:
- Pool exhausted successfully
- New guests cannot connect
- Alert triggered at 80% utilization

Corporate Network:
- No impact observed
- VLAN isolation effective
- Services remain available
```

### Success Criteria
- [x] Guest network impact isolated
- [x] Corporate network protected
- [x] Alerts functioning
- [x] Recovery time < 5 minutes

---

## Tips and Best Practices

### Before Testing
1. **Always** get written authorization
2. Use a test network when possible
3. Have a rollback plan
4. Set time limits
5. Monitor continuously

### During Testing
1. Start with short duration tests
2. Monitor impact continuously
3. Be ready to stop immediately
4. Document everything
5. Communicate with stakeholders

### After Testing
1. Verify normal operation restored
2. Review logs thoroughly
3. Document findings
4. Provide recommendations
5. Schedule follow-up

### Troubleshooting

#### Attack Not Working
- Check WiFi password is correct
- Verify target network is reachable
- Confirm DHCP server is responding
- Check for security controls blocking attack

#### Web Interface Not Accessible
- Verify connected to "Sinkhole" WiFi
- Try http://10.13.37.1 directly
- Check serial console for errors
- Restart device if needed

#### Device Disconnecting
- Check power supply
- Verify antenna connection
- Reduce distance to target AP
- Check serial console for errors

---

## Real-World Results

### Typical Findings

**Home Routers** (Consumer Grade)
- Pool Size: 50-254 addresses
- Time to Exhaust: 2-10 minutes
- Mitigations: Usually none
- Impact: Complete service denial

**Small Business**
- Pool Size: 100-500 addresses
- Time to Exhaust: 10-30 minutes
- Mitigations: Rare
- Impact: Significant disruption

**Enterprise** (Without Protection)
- Pool Size: 500-5000 addresses
- Time to Exhaust: 30-120 minutes
- Mitigations: Sometimes present
- Impact: Depends on pool usage

**Enterprise** (With Protection)
- Attack detected and blocked
- Rate limiting effective
- Alerts triggered
- Minimal impact

---

## Conclusion

These scenarios demonstrate various use cases for the DHCP Depleter tool. Always remember to:

1. Obtain proper authorization
2. Test responsibly
3. Document thoroughly
4. Implement fixes
5. Retest to verify

For more information, see:
- [README.md](README.md) - General overview
- [SECURITY.md](SECURITY.md) - Legal and ethical guidelines
- [BUILDING.md](BUILDING.md) - Build instructions
