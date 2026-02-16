# UniversalJumpscare Changelog
## v1.1.1 and v1.1.2
- Port to GD 2.2081.
## v1.1.0
- Added Jumpscare Randomization.
  - Register jumpscares in one of three ways:
    - An image file with no corresponding audio file (ex:  `jumpscare.png`  )
    - An image file with an audio file sharing the same name (ex:  `jumpscareOne.gif`  +  `jumpscareOne.mp3`  )
    - A subfolder containing the files  `jumpscare.png`  for the jumpscare image, and  `jumpscareAudio.mp3`  for the jumpscare audio (this third option is specifically for people who also use Weebify's Jumpscare mod, which only happens when playing levels)
  - Jumpscares are selected randomly after the current jumpscare's image portion finishes playing and fades out.
  - You have two folders of jumpscares to choose from. You can choose to get jumpscares from one folder or no folders.
  - Try to keep your jumpscare files small, because jumpscare images can be loaded any time!
  - To reload your custom jumpscare files, find the "Click Me to Reload Jumpscares" button in the mod settings. 
- Added a new option to change how often jumpscare probabilities are calculated.
  - In case the already low probabilities from the probability denominator setting aren't low enough.
  - Options range from 0.01667 (equivalent to 1/60) to 1000 seconds.
## v1.0.1
- Meaningless mod metadata modification. Move along.
## v1.0.0
- Initial release (on GitHub).