/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTADVANCEDCHOKEALGORITHM_H
#define BTADVANCEDCHOKEALGORITHM_H

#include <peer/peer.h>
#include "choker.h"


namespace bt
{
	struct TorrentStats;


	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class AdvancedChokeAlgorithm : public ChokeAlgorithm
	{
	public:
		AdvancedChokeAlgorithm();
		virtual ~AdvancedChokeAlgorithm();

		virtual void doChokingLeechingState(PeerManager & pman, ChunkManager & cman, const TorrentStats & stats);
		virtual void doChokingSeedingState(PeerManager & pman, ChunkManager & cman, const TorrentStats & stats);

	private:
		bool calcACAScore(Peer::Ptr p, ChunkManager & cman, const TorrentStats & stats);
		Peer::Ptr updateOptimisticPeer(PeerManager & pman, const QList<Peer::Ptr> & ppl);
		void doUnchoking(QList<Peer::Ptr> & ppl, Peer::Ptr poup);
		
	private:
		TimeStamp last_opt_sel_time; // last time we updated the optimistic unchoked peer
	};

}

#endif
