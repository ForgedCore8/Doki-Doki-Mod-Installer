function checkChanged() {
    var checkbox = document.getElementById('newDirCheckbox');
    var modPathGroup = document.getElementById('modPathGroup');
    if (checkbox.checked) {
        modPathGroup.style.display = 'block';
    } else {
        modPathGroup.style.display = 'none';
        document.getElementById('modPathEntry').value = ''; // Clear the input
    }
}


eel.expose(disableUIElements);
function disableUIElements() {
    // Disable buttons and inputs
    document.getElementById('zipBrowse').disabled = true;
    document.getElementById('gamePathBrowse').disabled = true;
    document.getElementById('autoButton').disabled = true;
    document.getElementById('processButton').disabled = true;
    document.getElementById('deleteButton').disabled = true;
}

eel.expose(showProgressBar)
function showProgressBar(value) {
    // Show and update the progress bar. Value is expected to be between 0 and 100.
    let progressBar = document.getElementById('progressBar');
    progressBar.value = value; // Update progress bar value
    progressBar.style.display = 'block'; // Make sure the progress bar is visible
}

eel.expose(enableUIElements);
function enableUIElements() {
    // Enable buttons and inputs
    document.getElementById('zipBrowse').disabled = false;
    document.getElementById('gamePathBrowse').disabled = false;
    document.getElementById('autoButton').disabled = false;
    document.getElementById('processButton').disabled = false;
    document.getElementById('deleteButton').disabled = false;
}

function onButtonClick() {
    var checkbox = document.getElementById('newDirCheckbox');
    var zipPath = document.getElementById('zipEntry').value;
    var gamePath = document.getElementById('gamePathEntry').value;
    var separateModPath = checkbox.checked ? document.getElementById('modPathEntry').value : null;
    console.log(separateModPath)
    if (!zipPath || !gamePath) {
        showMessage('Error', "Please specify both the ZIP file and the game directory.")
        return
    }
    showProgressBar()
    disableUIElements()
        eel.process_files(zipPath, gamePath, separateModPath)
}

function browsePath(entry, isFolder) {
    eel.browse_path(isFolder)(function (result) {
        if (result) {
            document.getElementById(entry).value = result.path
            appendToConsole(result.action + result.path);
        }
    });
}

function autoToggle() {
    // Example of toggling auto-detection, implementing the actual auto-detection requires Python backend
    eel.auto_toggle()(function (result) {
        appendToConsole(result);
        document.getElementById('gamePathEntry').value = result.gamePath || ''; // Update the game path based on Python's response
    });
}

eel.expose(appendToConsole);
function appendToConsole(message) {
    const consoleOutput = document.getElementById('consoleOutput');
    // Create a new div for each message
    const messageElement = document.createElement('div');
    messageElement.textContent = message;
    // Append the new message div to the console
    consoleOutput.appendChild(messageElement);
    // Scroll to the bottom
    consoleOutput.scrollTop = consoleOutput.scrollHeight;
}




eel.expose(showMessage);
function showMessage(title, message) {
    document.getElementById('messageBoxTitle').textContent = title;
    document.getElementById('messageBoxText').textContent = message;
    document.getElementById('messageBox').classList.remove('hidden');
}

function closeMessageBox() {
    document.getElementById('messageBox').classList.add('hidden');
}

addEventListener("DOMContentLoaded", () => {
    appendToConsole("Select a ZIP file to Install")
});

eel.expose(setProgress);
function setProgress(percent) {
    var progressBar = document.getElementById('progressBar');
    progressBar.style.width = percent + '%';
}

eel.expose(updateProgressBar);
function updateProgressBar(value) {
    var progressBar = document.getElementById('progressBar');
    progressBar.style.width = value + '%';
}