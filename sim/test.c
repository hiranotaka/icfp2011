#include "sim.h"
#include "debug.h"

int main(int argc, char* argv[])
{
	struct game *game = create_game();

	apply_sc(0, &zero_value, game); switch_turn(game);

	apply_sc(100, &help_value, game); switch_turn(game);
	apply_cs(&K_value, 100, game); switch_turn(game);
	apply_cs(&S_value, 100, game); switch_turn(game);
	apply_sc(100, &get_value, game); switch_turn(game);
	apply_sc(100, &zero_value, game); switch_turn(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);

	apply_cs(&K_value, 100, game); switch_turn(game);
	apply_cs(&S_value, 100, game); switch_turn(game);
	apply_sc(100, &get_value, game); switch_turn(game);
	apply_sc(100, &zero_value, game); switch_turn(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game);  switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game); // 8192

	apply_cs(&K_value, 100, game); switch_turn(game);
	apply_cs(&S_value, 100, game); switch_turn(game);
	apply_sc(100, &get_value, game); switch_turn(game);
	apply_sc(100, &zero_value, game); switch_turn(game);

	print_game(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);

	apply_sc(100, &attack_value, game); switch_turn(game);
	apply_cs(&K_value, 100, game); switch_turn(game);
	apply_cs(&S_value, 100, game); switch_turn(game);
	apply_sc(100, &get_value, game); switch_turn(game);
	apply_sc(100, &zero_value, game); switch_turn(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);

	apply_cs(&K_value, 100, game); switch_turn(game);
	apply_cs(&S_value, 100, game); switch_turn(game);
	apply_sc(100, &get_value, game); switch_turn(game);
	apply_sc(100, &zero_value, game); switch_turn(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game);  switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game); // 16384

	apply_cs(&K_value, 100, game); switch_turn(game);
	apply_cs(&S_value, 100, game); switch_turn(game);
	apply_sc(100, &get_value, game); switch_turn(game);
	apply_sc(100, &zero_value, game);

	print_game(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);
	apply_cs(&copy_value, 0, game); switch_turn(game);

	print_game(game);

	apply_cs(&put_value, 0, game); switch_turn(game);
	apply_sc(0, &zero_value, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&dbl_value, 0, game); switch_turn(game);
	apply_cs(&succ_value, 0, game); switch_turn(game);
	apply_cs(&revive_value, 0, game); switch_turn(game);

	print_game(game);

	destroy_game(game);

	return 0;
}
