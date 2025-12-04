import { invoke } from '@tauri-apps/api/core';
import { getCurrentWindow } from '@tauri-apps/api/window';
import { EditorView } from 'codemirror';
import { EditorState } from '@codemirror/state';
import { keymap, highlightActiveLine, drawSelection, rectangularSelection, crosshairCursor, highlightSpecialChars } from '@codemirror/view';
import { defaultKeymap, history, historyKeymap } from '@codemirror/commands';
import { bracketMatching } from '@codemirror/language';
import { vim } from '@replit/codemirror-vim';

// State
let config = null;
let tests = [];
let currentTestIndex = -1;
let inputEditor = null;
let outputEditor = null;
let deletedTest = null; // Store last deleted test for undo
let deletedTestIndex = -1; // Store index where test was deleted

// Theme will be set after getting from Rust backend
let isDarkMode = false;

// Initialize theme from system
async function initTheme() {
  try {
    const theme = await invoke('get_system_theme');
    isDarkMode = theme === 'dark';
    
    // Apply theme class to body for CSS
    document.body.classList.toggle('dark-theme', isDarkMode);
    document.body.classList.toggle('light-theme', !isDarkMode);
  } catch (e) {
    console.error('Failed to get system theme:', e);
    // Fallback to CSS media query
    isDarkMode = window.matchMedia('(prefers-color-scheme: dark)').matches;
  }
}

// Get CSS variable values
const getComputedColor = (varName) => {
  return getComputedStyle(document.documentElement).getPropertyValue(varName).trim();
};

// CodeMirror base theme - common styles
const editorTheme = EditorView.theme({
  '&': {
    height: '100%',
    fontSize: '12px',
  },
  '.cm-scroller': {
    overflow: 'auto',
    fontFamily: "'SF Mono', Monaco, 'Cascadia Code', monospace",
  },
  '.cm-content': {
    minHeight: '100px',
  },
  '&.cm-focused': {
    outline: 'none',
  },
}, { dark: isDarkMode });

// Custom light theme with Monaco-like selection color
const lightTheme = EditorView.theme({
  '&': {
    backgroundColor: '#ffffff',
    color: '#333333',
  },
  '.cm-content': {
    caretColor: '#000000',
    padding: '4px 0',
  },
  '.cm-cursor': {
    borderLeftColor: '#000000',
    borderLeftWidth: '2px',
  },
  '&.cm-focused .cm-selectionBackground, .cm-selectionBackground': {
    backgroundColor: '#add6ff',  // Monaco light selection blue
  },
  '.cm-activeLine': {
    backgroundColor: '#f5f5f5',
  },
}, { dark: false });

// Custom dark theme with Monaco-like dark selection color
const darkTheme = EditorView.theme({
  '&': {
    backgroundColor: '#1e1e1e',
    color: '#d4d4d4',
  },
  '.cm-content': {
    caretColor: '#aeafad',
    padding: '4px 0',
  },
  '.cm-cursor': {
    borderLeftColor: '#aeafad',
    borderLeftWidth: '2px',
  },
  '&.cm-focused .cm-selectionBackground, .cm-selectionBackground': {
    backgroundColor: '#264f78',  // Monaco dark selection blue
  },
  '.cm-activeLine': {
    backgroundColor: '#2a2a2a',
  },
}, { dark: true });

// Create editor extensions (minimal setup without line numbers)
function getExtensions() {
  const extensions = [
    vim(),
    highlightSpecialChars(),
    history(),
    drawSelection(),
    EditorState.allowMultipleSelections.of(true),
    bracketMatching(),
    rectangularSelection(),
    crosshairCursor(),
    highlightActiveLine(),
    keymap.of([...defaultKeymap, ...historyKeymap]),
    editorTheme,
    EditorView.lineWrapping,
    isDarkMode ? darkTheme : lightTheme,
  ];
  
  return extensions;
}

// Create a CodeMirror editor
function createEditor(parent, initialContent = '') {
  const state = EditorState.create({
    doc: initialContent,
    extensions: getExtensions(),
  });
  
  return new EditorView({
    state,
    parent,
  });
}

// Get editor content
function getEditorContent(editor) {
  return editor ? editor.state.doc.toString() : '';
}

// Set editor content
function setEditorContent(editor, content) {
  if (editor) {
    editor.dispatch({
      changes: {
        from: 0,
        to: editor.state.doc.length,
        insert: content,
      },
    });
  }
}

// DOM Elements
const elements = {
  // Info panel
  taskName: document.getElementById('taskName'),
  groupName: document.getElementById('groupName'),
  interactive: document.getElementById('interactive'),

  // Test list
  testList: document.getElementById('testList'),
  testListContainer: document.getElementById('testListContainer'),
  btnAll: document.getElementById('btnAll'),
  btnNew: document.getElementById('btnNew'),
  btnRemove: document.getElementById('btnRemove'),
  btnSave: document.getElementById('btnSave'),

  // Test editor containers
  testInputContainer: document.getElementById('testInput'),
  testOutputContainer: document.getElementById('testOutput'),
  knowAnswer: document.getElementById('knowAnswer'),
  outputContainer: document.getElementById('outputContainer'),

  // Options
  timeLimit: document.getElementById('timeLimit'),
  checker: document.getElementById('checker'),
  truncateLongTest: document.getElementById('truncateLongTest'),
  hideAcceptedTest: document.getElementById('hideAcceptedTest'),
  stopAtWrongAnswer: document.getElementById('stopAtWrongAnswer'),
  knowGenAns: document.getElementById('knowGenAns'),

  // Generator
  useGeneration: document.getElementById('useGeneration'),
  genOptions: document.getElementById('genOptions'),
  numTest: document.getElementById('numTest'),
  generatorSeed: document.getElementById('generatorSeed'),
  genParameters: document.getElementById('genParameters'),

  // Override
  overrideSolution: document.getElementById('overrideSolution'),
  overrideSlow: document.getElementById('overrideSlow'),
  overrideGen: document.getElementById('overrideGen'),
};

// Initialize the app
async function init() {
  // Get system theme first (before creating editors)
  await initTheme();
  
  // Initialize CodeMirror editors (after theme is set)
  inputEditor = createEditor(elements.testInputContainer, '');
  outputEditor = createEditor(elements.testOutputContainer, '');
  
  try {
    config = await invoke('load_config');
    tests = config.tests || [];
    populateForm();
    renderTestList();
    if (tests.length > 0) {
      selectTest(0);
    }
  } catch (error) {
    console.error('Failed to load config:', error);
  }

  setupEventListeners();
}

function populateForm() {
  elements.taskName.value = config.name || '';
  elements.groupName.value = config.group || '';
  elements.interactive.checked = config.interactive || false;

  elements.timeLimit.value = config.timeLimit || 0;
  elements.checker.value = config.checker || 'token_checker';
  elements.truncateLongTest.checked = config.truncateLongTest || false;
  elements.hideAcceptedTest.checked = config.hideAcceptedTest || false;
  elements.stopAtWrongAnswer.checked = config.stopAtWrongAnswer || false;
  elements.knowGenAns.checked = config.knowGenAns || false;

  elements.useGeneration.checked = config.useGeneration || false;
  elements.genOptions.classList.toggle('hidden', !config.useGeneration);
  elements.numTest.value = config.numTest || 0;
  elements.generatorSeed.value = config.generatorSeed || '';
  elements.genParameters.value = config.genParameters || '';

  const langConfig = config.languageConfig || {};
  elements.overrideSolution.value = langConfig.solution || '';
  elements.overrideSlow.value = langConfig.slow || '';
  elements.overrideGen.value = langConfig.gen || '';
}

function renderTestList() {
  elements.testList.innerHTML = '';
  tests.forEach((test, index) => {
    const item = document.createElement('div');
    item.className = `test-item ${index === currentTestIndex ? 'selected' : ''}`;
    item.innerHTML = `
      <input type="checkbox" ${test.active ? 'checked' : ''} data-index="${index}" />
      <span>${formatTestLabel(test, index)}</span>
    `;
    item.addEventListener('click', (e) => {
      if (e.target.type !== 'checkbox') {
        saveCurrentTest();
        selectTest(index);
      }
    });
    item.querySelector('input').addEventListener('change', (e) => {
      tests[index].active = e.target.checked;
    });
    elements.testList.appendChild(item);
  });
}

function formatTestLabel(test, index) {
  let inputPreview = (test.input || '').replace(/\n/g, ' ');
  if (inputPreview.length > 15) {
    inputPreview = inputPreview.substring(0, 12) + '...';
  }
  return `Test #${index + 1}: ${inputPreview}`;
}

function selectTest(index) {
  currentTestIndex = index;
  if (index >= 0 && index < tests.length) {
    const test = tests[index];
    setEditorContent(inputEditor, test.input || '');
    setEditorContent(outputEditor, test.output || '');
    elements.knowAnswer.checked = test.answer !== false;
    elements.outputContainer.classList.toggle('hidden', !elements.knowAnswer.checked);
  }
  renderTestList();
}

function saveCurrentTest() {
  if (currentTestIndex >= 0 && currentTestIndex < tests.length) {
    tests[currentTestIndex] = {
      ...tests[currentTestIndex],
      input: getEditorContent(inputEditor),
      output: getEditorContent(outputEditor),
      answer: elements.knowAnswer.checked,
    };
  }
}

function setupEventListeners() {
  // Test list buttons
  elements.btnAll.addEventListener('click', () => {
    const allSelected = tests.every((test) => test.active);
    tests.forEach((test) => (test.active = !allSelected));
    renderTestList();
  });

  elements.btnNew.addEventListener('click', () => {
    saveCurrentTest();
    const newTest = {
      input: '',
      output: '',
      index: tests.length,
      active: true,
      answer: true,
    };
    tests.push(newTest);
    selectTest(tests.length - 1);
  });

  elements.btnRemove.addEventListener('click', () => {
    if (currentTestIndex >= 0 && tests.length > 0) {
      // Save current editor content to the test before deleting
      saveCurrentTest();
      
      // Store deleted test for undo (now with latest content)
      deletedTest = { ...tests[currentTestIndex] };
      deletedTestIndex = currentTestIndex;
      
      tests.splice(currentTestIndex, 1);
      // Re-index tests
      tests.forEach((test, i) => (test.index = i));
      if (currentTestIndex >= tests.length) {
        currentTestIndex = tests.length - 1;
      }
      if (tests.length > 0) {
        selectTest(currentTestIndex);
      } else {
        currentTestIndex = -1;
        setEditorContent(inputEditor, '');
        setEditorContent(outputEditor, '');
        renderTestList();
      }
    }
  });

  // Know answer checkbox
  elements.knowAnswer.addEventListener('change', () => {
    elements.outputContainer.classList.toggle('hidden', !elements.knowAnswer.checked);
    saveCurrentTest();
  });

  // Generate test checkbox
  elements.useGeneration.addEventListener('change', () => {
    elements.genOptions.classList.toggle('hidden', !elements.useGeneration.checked);
  });

  // Save button
  elements.btnSave.addEventListener('click', saveConfig);

  // Test list keyboard navigation
  elements.testListContainer.addEventListener('keydown', (e) => {
    if (e.key === 'ArrowDown' || e.key === 'j') {
      e.preventDefault();
      e.stopPropagation();
      if (currentTestIndex < tests.length - 1) {
        saveCurrentTest();
        selectTest(currentTestIndex + 1);
      }
    } else if (e.key === 'ArrowUp' || e.key === 'k') {
      e.preventDefault();
      e.stopPropagation();
      if (currentTestIndex > 0) {
        saveCurrentTest();
        selectTest(currentTestIndex - 1);
      }
    } else if (e.key === 'Enter') {
      e.preventDefault();
      e.stopPropagation();
      inputEditor?.focus();
    } else if (e.key === ' ') {
      e.preventDefault();
      e.stopPropagation();
      if (currentTestIndex >= 0) {
        tests[currentTestIndex].active = !tests[currentTestIndex].active;
        renderTestList();
      }
    }
  });

  // Global keyboard shortcuts
  document.addEventListener('keydown', (e) => {
    const ctrl = e.metaKey || e.ctrlKey;
    const isInEditor = inputEditor?.hasFocus || outputEditor?.hasFocus;
    const isInInput = document.activeElement?.tagName === 'INPUT' || document.activeElement?.tagName === 'SELECT';
    const isInTestList = document.activeElement === elements.testListContainer;
    const isFree = !isInEditor && !isInInput; // Not focused on any text input
    
    // Escape - quit without save when at top level (test list focused or nothing focused)
    if (e.key === 'Escape') {
      e.preventDefault();
      if (isInEditor || isInInput) {
        // If in editor/input, just blur and focus test list
        document.activeElement?.blur();
        elements.testListContainer.focus();
      } else {
        // At top level - close without saving
        getCurrentWindow().close();
      }
      return;
    }

    // Space to toggle current test active (when not in editor/input)
    if (e.key === ' ' && isFree) {
      e.preventDefault();
      if (currentTestIndex >= 0) {
        tests[currentTestIndex].active = !tests[currentTestIndex].active;
        renderTestList();
      }
      return;
    }

    // Ctrl+S to save (always works)
    if (ctrl && e.key.toLowerCase() === 's') {
      e.preventDefault();
      saveConfig();
      return;
    }

    // When NOT in editor/input, use single keys
    if (isFree) {
      // Number keys 1-9 for test selection
      const keyNum = parseInt(e.key);
      if (!isNaN(keyNum) && keyNum >= 1 && keyNum <= 9 && keyNum <= tests.length) {
        e.preventDefault();
        saveCurrentTest();
        selectTest(keyNum - 1);
        return;
      }

      switch (e.key.toLowerCase()) {
        case 'n': // New test
          e.preventDefault();
          elements.btnNew.click();
          break;
        case 'd': // Delete test
          e.preventDefault();
          elements.btnRemove.click();
          break;
        case 'u': // Undo deleted test
          e.preventDefault();
          if (deletedTest) {
            saveCurrentTest();
            // Insert at original position
            tests.splice(deletedTestIndex, 0, deletedTest);
            // Re-index tests
            tests.forEach((test, i) => (test.index = i));
            selectTest(deletedTestIndex);
            deletedTest = null;
            deletedTestIndex = -1;
          }
          break;
        case 'k': // Previous test (up)
          e.preventDefault();
          if (currentTestIndex > 0) {
            saveCurrentTest();
            selectTest(currentTestIndex - 1);
          }
          break;
        case 'j': // Next test (down)
          e.preventDefault();
          if (currentTestIndex < tests.length - 1) {
            saveCurrentTest();
            selectTest(currentTestIndex + 1);
          }
          break;
        case 'e': // Toggle Expected answer
          e.preventDefault();
          elements.knowAnswer.checked = !elements.knowAnswer.checked;
          elements.knowAnswer.dispatchEvent(new Event('change'));
          break;
        case 'i': // Focus Input editor
          e.preventDefault();
          inputEditor?.focus();
          break;
        case 'o': // Focus Output editor
          e.preventDefault();
          outputEditor?.focus();
          break;
        case 't': // Focus time limit
          e.preventDefault();
          elements.timeLimit.focus();
          elements.timeLimit.select();
          break;
        case 'w': // Toggle stop on WA
          e.preventDefault();
          elements.stopAtWrongAnswer.checked = !elements.stopAtWrongAnswer.checked;
          break;
        case 'g': // Toggle know generator answer
          e.preventDefault();
          elements.knowGenAns.checked = !elements.knowGenAns.checked;
          break;
        case 'c': // Focus checker dropdown
          e.preventDefault();
          elements.checker.focus();
          break;
        case 'r': // Toggle truncate
          e.preventDefault();
          elements.truncateLongTest.checked = !elements.truncateLongTest.checked;
          break;
        case 'h': // Toggle hide accepted
          e.preventDefault();
          elements.hideAcceptedTest.checked = !elements.hideAcceptedTest.checked;
          break;
        case 'y': // Toggle interactYve
          e.preventDefault();
          elements.interactive.checked = !elements.interactive.checked;
          break;
        case 'a': // Toggle All/None tests
          e.preventDefault();
          const allSelected = tests.every((test) => test.active);
          tests.forEach((test) => (test.active = !allSelected));
          renderTestList();
          break;
      }
    }
  });
}

async function saveConfig() {
  saveCurrentTest();

  const updatedConfig = {
    name: elements.taskName.value,
    group: elements.groupName.value,
    interactive: elements.interactive.checked,
    timeLimit: parseInt(elements.timeLimit.value) || 0,
    checker: elements.checker.value,
    truncateLongTest: elements.truncateLongTest.checked,
    hideAcceptedTest: elements.hideAcceptedTest.checked,
    stopAtWrongAnswer: elements.stopAtWrongAnswer.checked,
    knowGenAns: elements.knowGenAns.checked,
    useGeneration: elements.useGeneration.checked,
    numTest: parseInt(elements.numTest.value) || 0,
    generatorSeed: elements.generatorSeed.value,
    genParameters: elements.genParameters.value,
    tests: tests,
    languageConfig: {
      solution: elements.overrideSolution.value || null,
      slow: elements.overrideSlow.value || null,
      gen: elements.overrideGen.value || null,
    },
  };

  try {
    await invoke('save_config', { config: updatedConfig });
    const appWindow = getCurrentWindow();
    await appWindow.close();
  } catch (error) {
    console.error('Failed to save config:', error);
    alert('Failed to save configuration: ' + error);
  }
}

// Start the app
document.addEventListener('DOMContentLoaded', init);
