#include "GameManager.hh"

#include "Global.hh"
#include "LegalMoves.hh"
#include "Pieces.hh"

namespace
{
	void updateSide(SideState &side, User defender, int homeRank)
	{
		side.inCheck = false;
		side.canCastleKingSide = !side.kingMoved && !side.kingSideRookMoved;
		side.canCastleQueenSide = !side.kingMoved && !side.queenSideRookMoved;

		const User attackerUser = opponentOf(defender);

		for (const Piece &attacker : Pieces::all())
		{
			if (attacker.user != attackerUser)
				continue;

			for (const Square &s : LegalMove::pseudoLegal(attacker))
			{
				// see checks
				if (s.piece.type == PieceType::King)
				{
					side.inCheck = true;
					side.canCastleKingSide = false;
					side.canCastleQueenSide = false;
				}

				// the king can't castle through attacked squares
				if (s.y == homeRank)
				{
					if (s.x == 5 || s.x == 6)
						side.canCastleKingSide = false;
					if (s.x == 1 || s.x == 2)
						side.canCastleQueenSide = false;
				}
			}
		}
	}
}

void GameManager::update()
{
	updateSide(Global.player, User::Player, 7);
	updateSide(Global.engine, User::Engine, 0);
}
