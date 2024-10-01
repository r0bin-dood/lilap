// counter.js

// Function to initialize and start the counter
function startCounter() {
    // Get the counter element
    const counterElement = document.getElementById('counter');

    // Initialize the count
    let count = 0;

    // Function to update the count
    function updateCount() {
        count++;
        if (count > 10) {
            count = 0; // Reset the count if it exceeds 10
        }
        counterElement.textContent = count; // Update the displayed count
    }

    // Update the count every second
    setInterval(updateCount, 1000); // 1000 ms = 1 second
}

// Wait for the DOM to be fully loaded before starting the counter
document.addEventListener('DOMContentLoaded', startCounter);
