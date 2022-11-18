import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.text.Document;
import javax.swing.undo.CannotRedoException;
import javax.swing.undo.CannotUndoException;
import javax.swing.undo.UndoManager;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Egor Kulikov (kulikov@devexperts.com)
 */
public class TaskConfigEditor extends JDialog {
    private static final int HEIGHT = new JLabel("Test").getPreferredSize().height;
    private final List<Test> tests;
    private int currentTest;
    private final JList testList;
    private final JTextArea input;
    private final JTextArea output;
    private final List<JCheckBox> checkBoxes = new ArrayList<>();
    private final JPanel checkBoxesPanel;
    private final JCheckBox knowAnswer;
    private final JPanel outputPanel;
    private boolean updating = false;

    public TaskConfigEditor(ProblemConfig problemConfig, String path) {
        super(null, "Tests", ModalityType.APPLICATION_MODAL);
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        this.tests = new ArrayList<>(problemConfig.tests);

        setAlwaysOnTop(true);
        setResizable(false);

        // #################################################################################################
        // # INFO #
        // #################################################################################################
        VariableGridLayout infoLayout = new VariableGridLayout(1, 3, 5, 5);
        infoLayout.setColFraction(0, 0.25);
        infoLayout.setColFraction(1, 0.5);

        JPanel infoPanel = new JPanel(infoLayout);

        // -------------------------------- task name --------------------------------
        JPanel taskNamePanel = new JPanel(new BorderLayout());
        taskNamePanel.add(new JLabel("Task name: "), BorderLayout.NORTH);
        JTextField taskNameTextField = new JTextField(10);
        taskNameTextField.setText(String.valueOf(problemConfig.name));
        taskNameTextField.setFont(Font.decode(Font.MONOSPACED));
        taskNamePanel.add(taskNameTextField);
        infoPanel.add(taskNamePanel);
        // -------------------------------- task name --------------------------------

        // -------------------------------- group name --------------------------------
        JPanel groupNamePanel = new JPanel(new BorderLayout());
        groupNamePanel.add(new JLabel("Group name: "), BorderLayout.NORTH);
        JTextField groupNameTextField = new JTextField(10);
        groupNameTextField.setText(String.valueOf(problemConfig.group));
        groupNameTextField.setFont(Font.decode(Font.MONOSPACED));
        groupNamePanel.add(groupNameTextField);
        infoPanel.add(groupNamePanel);
        // -------------------------------- group name --------------------------------

        // -------------------------------- interactive? ------------------------------
        JCheckBox interactiveCheckBox = new JCheckBox("Interactive Task?");
        interactiveCheckBox.setSelected(problemConfig.interactive);
        infoPanel.add(interactiveCheckBox);
        // -------------------------------- interactive --------------------------------

        // #################################################################################################
        // # MAIN #
        // #################################################################################################
        VariableGridLayout mainLayout = new VariableGridLayout(1, 3, 5, 5);
        mainLayout.setColFraction(0, 0.25);
        mainLayout.setColFraction(1, 0.50);

        JPanel mainPanel = new JPanel(mainLayout);

        JPanel selectorAndButtonsPanel = new JPanel(new BorderLayout());
        selectorAndButtonsPanel.add(new JLabel("Tests:"), BorderLayout.NORTH);
        JPanel checkBoxesAndSelectorPanel = new JPanel(new BorderLayout());

        checkBoxesPanel = new JPanel();
        checkBoxesPanel.setLayout(new BoxLayout(checkBoxesPanel, BoxLayout.Y_AXIS));
        for (Test test : tests) {
            JCheckBox checkBox = createCheckBox(test);
            checkBox.setSelected(test.active);
            checkBoxesPanel.add(checkBox);
        }
        checkBoxesAndSelectorPanel.add(checkBoxesPanel, BorderLayout.WEST);

        testList = new JList(tests.toArray());
        testList.setFixedCellHeight(HEIGHT);
        testList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        testList.setLayoutOrientation(JList.VERTICAL);
        testList.addListSelectionListener(e -> {
            if (updating) {
                return;
            }
            int index = testList.getSelectedIndex();
            ListModel model = testList.getModel();
            if (index >= 0 && index < model.getSize()) {
                saveCurrentTest();
                setSelectedTest(index);
            }
        });

        checkBoxesAndSelectorPanel.add(testList, BorderLayout.CENTER);
        selectorAndButtonsPanel.add(new JScrollPane(checkBoxesAndSelectorPanel,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER), BorderLayout.CENTER);
        JPanel buttonsPanel = new JPanel(new GridLayout(3, 1));
        JPanel upperButtonsPanel = new JPanel(new GridLayout(1, 2));

        JButton all = new JButton("All");
        all.addActionListener(e -> {
            int index = 0;
            for (JCheckBox checkBox : checkBoxes) {
                checkBox.setSelected(true);
                TaskConfigEditor.this.tests.set(index, TaskConfigEditor.this.tests.get(index).setActive(true));
                index++;
            }
            setSelectedTest(currentTest);
        });
        upperButtonsPanel.add(all);

        JButton none = new JButton("None");
        none.addActionListener(e -> {
            int index = 0;
            for (JCheckBox checkBox : checkBoxes) {
                checkBox.setSelected(false);
                TaskConfigEditor.this.tests.set(index, TaskConfigEditor.this.tests.get(index).setActive(false));
                index++;
            }
            setSelectedTest(currentTest);
        });
        upperButtonsPanel.add(none);
        buttonsPanel.add(upperButtonsPanel);
        JPanel middleButtonsPanel = new JPanel(new GridLayout(1, 2));
        JButton newTest = new JButton("New");
        newTest.addActionListener(e -> {
            saveCurrentTest();
            int index = TaskConfigEditor.this.tests.size();
            Test test = new Test("", "", index);
            TaskConfigEditor.this.tests.add(test);
            checkBoxesPanel.add(createCheckBox(test));
            setSelectedTest(index);
        });
        middleButtonsPanel.add(newTest);
        JButton remove = new JButton("Remove");
        remove.addActionListener(e -> {
            if (currentTest == -1) {
                return;
            }
            while (checkBoxes.size() > currentTest) {
                checkBoxesPanel.remove(checkBoxes.get(currentTest));
                checkBoxes.remove(currentTest);
            }
            TaskConfigEditor.this.tests.remove(currentTest);
            int size = TaskConfigEditor.this.tests.size();
            for (int i = currentTest; i < size; i++) {
                Test test = TaskConfigEditor.this.tests.get(i);
                test = new Test(test.input, test.output, i, test.active);
                TaskConfigEditor.this.tests.set(i, test);
                checkBoxesPanel.add(createCheckBox(test));
            }
            if (currentTest < size) {
                setSelectedTest(currentTest);
                return;
            }
            if (size > 0) {
                setSelectedTest(0);
                return;
            }
            setSelectedTest(-1);
        });
        middleButtonsPanel.add(remove);
        buttonsPanel.add(middleButtonsPanel);

        JPanel testPanel = new JPanel(new GridLayout(2, 1, 5, 5));
        JPanel inputPanel = new JPanel(new BorderLayout());
        inputPanel.add(new JLabel("Input:"), BorderLayout.NORTH);
        DocumentListener listener = new DocumentListener() {
            public void insertUpdate(DocumentEvent e) {
                saveCurrentTest();
            }

            public void removeUpdate(DocumentEvent e) {
                saveCurrentTest();
            }

            public void changedUpdate(DocumentEvent e) {
                saveCurrentTest();
            }
        };
        input = new JTextArea();
        input.setFont(Font.decode(Font.MONOSPACED));
        input.getDocument().addDocumentListener(listener);
        inputPanel.add(new JScrollPane(input), BorderLayout.CENTER);
        outputPanel = new JPanel(new BorderLayout());
        outputPanel.add(new JLabel("Output:"), BorderLayout.NORTH);
        output = new JTextArea();
        output.setFont(Font.decode(Font.MONOSPACED));
        output.getDocument().addDocumentListener(listener);
        outputPanel.add(new JScrollPane(output), BorderLayout.CENTER);
        knowAnswer = new JCheckBox("Know answer?");
        knowAnswer.addActionListener(e -> saveCurrentTest());
        JPanel outputAndCheckBoxPanel = new JPanel(new BorderLayout());
        outputAndCheckBoxPanel.add(knowAnswer, BorderLayout.NORTH);
        outputAndCheckBoxPanel.add(outputPanel, BorderLayout.CENTER);
        testPanel.add(inputPanel);
        testPanel.add(outputAndCheckBoxPanel);

        JPanel optionsPanel = new JPanel(new VariableGridLayout(7, 1, 5, 5));

        // -------------------------------- time limit --------------------------------
        JPanel timeLimitPanel = new JPanel(new BorderLayout());
        timeLimitPanel.add(new JLabel("Time Limit in ms:"), BorderLayout.NORTH);
        JTextField timeLimitTextField = new JTextField(10);
        timeLimitTextField.setText(String.valueOf(problemConfig.timeLimit));
        timeLimitTextField.setFont(Font.decode(Font.MONOSPACED));
        timeLimitPanel.add(timeLimitTextField);
        optionsPanel.add(timeLimitPanel);
        // -------------------------------- time limit --------------------------------

        // -------------------------------- checker --------------------------------
        JPanel checkerPanel = new JPanel(new BorderLayout());
        checkerPanel.add(new JLabel("Checker:"), BorderLayout.NORTH);
        JTextField checkerTextField = new JTextField(10);
        checkerTextField.setText(String.valueOf(problemConfig.checker));
        checkerTextField.setFont(Font.decode(Font.MONOSPACED));
        checkerPanel.add(checkerTextField);
        optionsPanel.add(checkerPanel);
        // -------------------------------- checker --------------------------------

        // -------------------------------- compact --------------------------------
        // JCheckBox compactCheckBox = new JCheckBox("Compact Mode?");
        // compactCheckBox.setSelected(compact);
        // optionsPanel.add(compactCheckBox);
        // -------------------------------- compact --------------------------------

        // -------------------------------- truncate long test
        // --------------------------------
        JCheckBox truncateLongTestCheckBox = new JCheckBox("Truncate Long Test?");
        truncateLongTestCheckBox.setSelected(problemConfig.truncateLongTest);
        optionsPanel.add(truncateLongTestCheckBox);
        // -------------------------------- truncate long test
        // --------------------------------

        // -------------------------------- hide accepted tests
        // --------------------------------
        JCheckBox hideAcceptedTestCheckBox = new JCheckBox("Hide Accepted Tests?");
        hideAcceptedTestCheckBox.setSelected(problemConfig.hideAcceptedTest);
        optionsPanel.add(hideAcceptedTestCheckBox);
        // -------------------------------- hide accepted tests
        // --------------------------------

        // -------------------------------- stop At Wrong Answer
        // --------------------------------
        JCheckBox stopAtWrongAnswerCheckBox = new JCheckBox("Stop At Wrong Answer?");
        stopAtWrongAnswerCheckBox.setSelected(problemConfig.stopAtWrongAnswer);
        optionsPanel.add(stopAtWrongAnswerCheckBox);
        // -------------------------------- stop At Wrong Answer
        // --------------------------------

        // -------------------------------- know generator answer
        // --------------------------------
        JCheckBox knowGenAnsCheckBox = new JCheckBox("Know generator answer?");
        knowGenAnsCheckBox.setSelected(problemConfig.knowGenAns);
        optionsPanel.add(knowGenAnsCheckBox);
        // -------------------------------- know generator answer
        // --------------------------------

        // #################################################################################################
        // # GEN #
        // #################################################################################################
        VariableGridLayout genLayout = new VariableGridLayout(2, 1, 5, 5);
        genLayout.setRowFraction(1, 0.8);
        JPanel gen = new JPanel(genLayout);

        JPanel genOptions = new JPanel(new VariableGridLayout(1, 4, 5, 5));

        // -------------------------------- num test --------------------------------
        JPanel numTestPanel = new JPanel(new BorderLayout());
        numTestPanel.add(new JLabel("Number of tests:"), BorderLayout.NORTH);
        JTextField numTestTextField = new JTextField(10);
        numTestTextField.setText(String.valueOf(problemConfig.numTest));
        numTestTextField.setFont(Font.decode(Font.MONOSPACED));
        numTestPanel.add(numTestTextField);
        genOptions.add(numTestPanel);
        // -------------------------------- num test --------------------------------

        // ----------------------------- generator seed -----------------------------
        JPanel generatorSeedPanel = new JPanel(new BorderLayout());
        generatorSeedPanel.add(new JLabel("Generator seed:"), BorderLayout.NORTH);
        JTextField generatorSeedTextField = new JTextField(10);
        generatorSeedTextField.setText(String.valueOf(problemConfig.generatorSeed));
        generatorSeedTextField.setFont(Font.decode(Font.MONOSPACED));
        generatorSeedPanel.add(generatorSeedTextField);
        genOptions.add(generatorSeedPanel);
        // ----------------------------- generator seed -----------------------------

        JPanel arg = new JPanel(new BorderLayout());
        arg.add(new JLabel("Generator argument:"), BorderLayout.NORTH);
        JTextField argF = new JTextField(problemConfig.genParameters);
        argF.setFont(Font.decode(Font.MONOSPACED));
        arg.add(argF);

        JPanel genPanel = new JPanel(new VariableGridLayout(2, 1, 5, 5));
        genPanel.add(genOptions);
        genPanel.add(arg);

        JCheckBox genCheckBox = new JCheckBox("Generate Test?");
        genCheckBox.setSelected(problemConfig.useGeneration);
        genCheckBox.addActionListener(e -> genPanel.setVisible(genCheckBox.isSelected()));

        genPanel.setVisible(problemConfig.useGeneration);
        gen.add(genCheckBox);
        gen.add(genPanel);

        VariableGridLayout finalL = new VariableGridLayout(3, 1, 5, 5);
        finalL.setRowFraction(0, 0.1);
        finalL.setRowFraction(1, 0.63);

        JPanel finalPanel = new JPanel(finalL);
        JButton save = new JButton("Save");
        save.addActionListener(e -> {
            saveCurrentTest();
            problemConfig.tests.clear();
            for (Test test : this.tests) {
                problemConfig.tests.add(test);
            }
            problemConfig.knowGenAns = knowGenAnsCheckBox.isSelected();
            problemConfig.useGeneration = genCheckBox.isSelected();
            problemConfig.truncateLongTest = truncateLongTestCheckBox.isSelected();
            problemConfig.stopAtWrongAnswer = stopAtWrongAnswerCheckBox.isSelected();
            problemConfig.hideAcceptedTest = hideAcceptedTestCheckBox.isSelected();
            problemConfig.interactive = interactiveCheckBox.isSelected();
            problemConfig.numTest = Long.parseLong("0" + numTestTextField.getText());
            problemConfig.timeLimit = Long.parseLong("0" +
                    timeLimitTextField.getText());
            problemConfig.genParameters = argF.getText();
            problemConfig.name = taskNameTextField.getText();
            problemConfig.group = groupNameTextField.getText();
            problemConfig.generatorSeed = generatorSeedTextField.getText();
            problemConfig.checker = checkerTextField.getText();
            try {
                FileWriter writer = new FileWriter(path);
                writer.write(gson.toJson(problemConfig));
                writer.flush();
                writer.close();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
            setVisible(false);
        });
        buttonsPanel.add(save);
        selectorAndButtonsPanel.add(buttonsPanel, BorderLayout.SOUTH);
        mainPanel.add(selectorAndButtonsPanel);
        mainPanel.add(testPanel);
        mainPanel.add(optionsPanel);

        finalPanel.add(infoPanel);
        finalPanel.add(mainPanel);
        finalPanel.add(gen);

        setContentPane(finalPanel);
        setSelectedTest(Math.min(0, tests.size() - 1));
        pack();

        // -------------------------------- center --------------------------------
        Toolkit it = Toolkit.getDefaultToolkit();
        Dimension d = it.getScreenSize();

        int W = 900, H = 550;
        setSize(W, H);
        setLocation(d.width / 2 - W / 2, d.height / 2 - H / 2);
    }

    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.out.println("Usage: java -jar TaskConfigEditor.jar <path/to/directory/with/config.json>");
            return;
        }
        Path config_path = Paths.get(args[0], "config.json").toAbsolutePath();
        Gson gson = new Gson();
        ProblemConfig problemConfig = gson.fromJson(new FileReader(config_path.toFile()), ProblemConfig.class);
        TaskConfigEditor dialog = new TaskConfigEditor(problemConfig, config_path.toString());
        dialog.setVisible(true);
        System.exit(0);
    }

    private JCheckBox createCheckBox(final Test test) {
        final JCheckBox checkBox = new JCheckBox("", test.active);
        Dimension preferredSize = new Dimension(checkBox.getPreferredSize().width, HEIGHT);
        checkBox.setPreferredSize(preferredSize);
        checkBox.setMaximumSize(preferredSize);
        checkBox.setMinimumSize(preferredSize);
        checkBox.addActionListener(e -> {
            tests.set(test.index, tests.get(test.index).setActive(checkBox.isSelected()));
            setSelectedTest(currentTest);
        });
        checkBoxes.add(checkBox);
        return checkBox;
    }

    private void setSelectedTest(int index) {
        updating = true;
        currentTest = -1;
        if (index == -1) {
            input.setVisible(false);
            output.setVisible(false);
        } else {
            addUndoRedoFunctionality(input);
            addUndoRedoFunctionality(output);
            input.setVisible(true);
            output.setVisible(true);
            input.setText(tests.get(index).input);
            knowAnswer.setSelected(tests.get(index).output != null);
            output.setText(knowAnswer.isSelected() ? tests.get(index).output : "");
            outputPanel.setVisible(knowAnswer.isSelected());
        }
        currentTest = index;
        testList.setListData(tests.toArray());
        if (testList.getSelectedIndex() != currentTest) {
            testList.setSelectedIndex(currentTest);
        }
        testList.repaint();
        checkBoxesPanel.repaint();
        updating = false;
    }

    private void addUndoRedoFunctionality(JTextArea textArea) {
        UndoManager undoManager = new UndoManager();

        Document document = textArea.getDocument();
        document.addUndoableEditListener(new UndoableEditListener() {
            @Override
            public void undoableEditHappened(UndoableEditEvent e) {
                undoManager.addEdit(e.getEdit());
            }
        });

        // Map undo action
        textArea.getInputMap(JComponent.WHEN_FOCUSED)
                .put(KeyStroke.getKeyStroke("meta Z"), "undoKeyStroke");
        textArea.getActionMap().put("undoKeyStroke", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    undoManager.undo();
                } catch (CannotUndoException cue) {
                }
            }
        });
        // Map redo action
        textArea.getInputMap(JComponent.WHEN_FOCUSED)
                .put(KeyStroke.getKeyStroke("meta shift Z"), "redoKeyStroke");
        textArea.getActionMap().put("redoKeyStroke", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    undoManager.redo();
                } catch (CannotRedoException cre) {
                }
            }
        });
    }

    private void saveCurrentTest() {
        if (currentTest == -1) {
            return;
        }
        tests.set(currentTest, new Test(input.getText(), knowAnswer.isSelected() ? output.getText() : null, currentTest,
                checkBoxes.get(currentTest).isSelected()));

        outputPanel.setVisible(knowAnswer.isSelected());
    }
}