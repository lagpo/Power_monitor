# Power_monitor
This built is a simulation of an industrial safety controller. Instead of a single "super loop" that runs line-by-line, the system mimics a real-world Electronic Control Unit (ECU) by running multiple independent threads (tasks) simultaneously on the ESP32.

The "Real-Life" Scenario Imagine a factory motor. One part of the computer needs to log its power usage for the cloud (slow, low priority), while another part needs to shut it down instantly if a human presses the emergency stop (fast, critical priority). The project replicates this architecture.

<img width="1093" height="215" alt="image" src="https://github.com/user-attachments/assets/8ab81e22-2e83-4912-bc9b-dc97dc1ca7e7" />
How Data Flows:

    Normal Op: SensorTask → [Queue] → ProcessTask → [Mutex] → Serial Monitor.

    Emergency: Button Press → ISR → [Semaphore] → SafetyTask (Preempts everything else!) → [Mutex] → Serial Monitor.

Why This Matters for Your Career

    Scalability: You can now add a WiFi task or a Display task without rewriting your sensor logic. You just add a new thread.

    Responsiveness: You learned that high-priority safety code runs immediately, even if the processor is busy doing math.

    Stability: You solved the "Garbled Text" problem, which is exactly how engineers solve memory corruption in complex drivers.

This project is a perfect "Hello World" to the professional embedded systems architecture used in automotive and aerospace industries.
