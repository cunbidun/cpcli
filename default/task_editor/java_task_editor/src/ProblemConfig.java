import java.util.List;

public class ProblemConfig {
    String genParameters;
    String name;
    String group;
    String checker;
    Long numTest;
    Long timeLimit;
    String generatorSeed;
    Boolean useGeneration;
    Boolean knowGenAns;
    Boolean hideAcceptedTest;
    Boolean truncateLongTest;
    Boolean stopAtWrongAnswer;
    Boolean interactive;
    List<Test> tests;
}