# devicetree entries
## (appended to <kernel-path>/arch/arm64/boot/dts/broadcom/bcm2711-rpi-4-b.dts)

```
&i2c1 {
	status = "okay";

	heart-rate@57 {
		compatible = "maxim,max30102";
		reg = <0x57>;
		interrupt-parent = <&gpio>;
		interrupts = <4 2>;
		maxim,red-led-current-microamp = <7000>;
		maxim,ir-led-current-microamp = <7000>;
	};
};
```

```
&gpio {
	max30102_int_pins: max30102-int-pins {
		pins = "gpio4";
		bias-pull-up;
	};
};
```
