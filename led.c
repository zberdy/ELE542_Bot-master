void statusChange(void)
{
	if ((led ^ PORTB) != 0)
	{
		
		PORTB = led;
	}
}
