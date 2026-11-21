package com.pokedaisy.app

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

/** The two-screen DS bottom display during a battle's bag or party: the touch screen up, and the player's own pick kept. */
class TouchRaiseTest {
    private val raise = TouchRaise()

    @Test
    fun theTouchScreenComesUpAndTheCompanionComesBack() {
        assertEquals(false, raise.update(up = true, showingCompanion = true))
        assertTrue(raise.raised)
        assertNull(raise.update(up = true, showingCompanion = false))
        assertEquals(true, raise.update(up = false, showingCompanion = false))
        assertFalse(raise.raised)
    }

    // Every sample says the bag is still up: a COMPANION tap meanwhile must not be undone by the next one.
    @Test
    fun theCompanionTheyPickedWhileTheBagIsUpStays() {
        raise.update(up = true, showingCompanion = true)
        raise.playerPicked()
        assertNull(raise.update(up = true, showingCompanion = true))
        assertNull(raise.update(up = false, showingCompanion = true))
    }

    // GAME SCREEN picked while raised: the companion isn't brought back over it when the bag closes.
    @Test
    fun theTouchScreenTheyPickedStaysAfterTheBag() {
        raise.update(up = true, showingCompanion = true)
        raise.playerPicked()
        assertNull(raise.update(up = false, showingCompanion = false))
    }

    @Test
    fun nothingWhenTheTouchScreenWasAlreadyUp() {
        assertNull(raise.update(up = true, showingCompanion = false))
        assertFalse(raise.raised)
        assertNull(raise.update(up = false, showingCompanion = false))
    }
}
