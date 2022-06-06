/**
 * @author Egor Kulikov (kulikov@devexperts.com)
 */
public class Test {
    public final int index;
    public final String input;
    public final String output;
    public final boolean active;


    public Test(String input, String output, long index) {
        this(input, output, (int) index, true);
    }

    public Test(String input, String output, long index, boolean active) {
        this.input = input;
        this.output = output;
        this.index = (int) index;
        this.active = active;
    }

    @Override
    public String toString() {
        String inputRepresentation = input.replace('\n', ' ');
        inputRepresentation = inputRepresentation.length() > 15 ? inputRepresentation.substring(0, 12) + "..." :
                inputRepresentation;
        return "Test #" + index + ": " + inputRepresentation;
    }

    public Test setActive(boolean active) {
        return new Test(input, output, index, active);
    }

}